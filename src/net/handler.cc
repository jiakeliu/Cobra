#include "debug.h"
#include "net.h"

cobraNetHandler::cobraNetHandler(QObject* parent, int cnt)
    :QTcpServer(parent), m_iThreadCnt(cnt), m_bServing(false)
{
    debug(LOW, "cobraNetHandler initializing...\n");
    m_idMine = NO_ID;
}

cobraNetHandler::~cobraNetHandler()
{
    debug(LOW, "cobraNetHandler dieing...\n");


    for (int x=0; x<m_tThreads.count(); x++) {
        if (!m_tThreads.at(x))
            continue;

        m_tThreads.at(x)->terminate();
        delete m_tThreads.at(x);
    }

    for (int x=0; x<m_cnetWorkers.count(); x++) {
        if (!m_cnetWorkers.at(x))
            continue;
        delete m_cnetWorkers.at(x);
    }

    QWriteLocker locker(&m_eventLock);

    cobraNetHandler::eventIterator iter = m_mihHandlers.begin();
    while (iter != m_mihHandlers.end()) {
        if (iter.value())
            iter.value()->put();
        iter.value() = NULL;
        iter++;
    }
    m_mihHandlers.clear();
}

cobraNetHandler*
cobraNetHandler::instance()
{
        static cobraNetHandler* static_cobra = NULL;
        if (!static_cobra) {
            static_cobra = new cobraNetHandler(QApplication::instance());
            static_cobra->moveToThread(QApplication::instance()->thread());
        }
        return static_cobra;
}

bool
cobraNetHandler::sendServerEvent(cobraNetEvent *event)
{
    if (!event)
        return false;

    debug(CRITICAL, "Attempting to send event\n");

    cobraId dest = event->destination();

    if (dest == BROADCAST) {
        return false;
    }

    QReadLocker locker(&m_cidLock);
    if (!m_cIds.contains(dest)) {
        debug(ERROR(CRITICAL), "Unknown Outbound Destination: %lu\n", (unsigned long)QThread::currentThreadId());
        return false;
    }

    int idx = m_cIds[dest];

    if (!m_cnetWorkers[idx]) {
        debug(ERROR(CRITICAL), "Failed to resolve the destination worker! (%d -> %d)\n", dest, idx);
        return false;
    }

    debug(CRITICAL, "Sending event...\n");

    qRegisterMetaType<cobraNetEvent*>("cobraNetEvent*");
    QMetaObject::invokeMethod(m_cnetWorkers[idx], "sendEvent", Qt::QueuedConnection, Q_ARG(cobraNetEvent*, event));

    return true;
}

bool
cobraNetHandler::broadcastServerEvent(cobraNetEvent *event)
{
    event->setDestination(BROADCAST);

    cobraNetHandler::workerIterator iter;
    for (iter=m_cnetWorkers.begin(); iter != m_cnetWorkers.end(); iter++) {
        if (!*iter)
            continue;

        qRegisterMetaType<cobraNetEvent*>("cobraNetEvent*");
        QMetaObject::invokeMethod(*iter, "sendEvent", Qt::QueuedConnection, Q_ARG(cobraNetEvent*, event));
    }

    return true;
}


bool
cobraNetHandler::event(QEvent* event)
{
    if (!event)
        return false;

    cobraNetEvent* cevent = dynamic_cast<cobraNetEvent*>(event);
    if (!cevent)
        return false;

    cobraNetEvent* pevent = cevent->duplicate();

    if (pevent->isRequest() && !isServing()) {
        debug(CRITICAL, "Sending event to Server!\n");
        return sendServerEvent(pevent);
    }

    /* If this is a broadcast message, fall through: we want it locally as well! */
    if (isServing()
            && pevent->isResponse()
            && pevent->source() == SERVER
            && pevent->destination() == BROADCAST) {
        debug(CRITICAL, "Broadcast message for send!\n");
        broadcastServerEvent(pevent);

        /* Update the previous event to go to our chat client */
        cevent->setDestination(m_idMine);
        cevent->setSource(SERVER);
        pevent = cevent;
    }

    cobraNetEventHandler* handler = getEventHandler(pevent->type());
    if (!handler) {
        debug(ERROR(CRITICAL), "Failed to find handler for event type! (%d)\n", pevent->type());
        return false;
    }

    handler->handleEvent(pevent);
    handler->put();

    debug(LOW, "Event: %d\n", pevent->type());
    return true;
}

bool
cobraNetHandler::eventFilter(QObject *object, QEvent * event)
{
    if (!object || !event)
        return false;

    if (object != this)
        return false;

    if (event->type() < cobraNetEventType)
        return false;

    debug(LOW, "Event Filter: %d\n", event->type());
    return cobraNetHandler::event(event);
}

bool
cobraNetHandler::registerEventHandler(cobraNetEventHandler* handler)
{
    if (!handler)
        return false;

    QWriteLocker locker(&m_eventLock);

    handler->get();
    if (m_mihHandlers.contains(handler->type()))
        return false;

    m_mihHandlers[handler->type()] = handler;

    debug(LOW, "Handler (%s,%d) Registered\n", CSTR(handler->name()), handler->type());
    return true;
}

cobraNetEventHandler*
cobraNetHandler::getEventHandler(int type)
{
    if (type <= cobraNetEventType)
        return NULL;

    QReadLocker locker(&m_eventLock);

    if (m_mihHandlers.contains(type))
        debug(LOW, "Handler (%s,%d) Retrieved\n", CSTR(m_mihHandlers[type]->name()), m_mihHandlers[type]->type());
    else
        debug(LOW, "NULL Handler Retrieved (%d)\n", type);

    cobraNetEventHandler* handler = m_mihHandlers[type];
    if (!handler)
        return NULL;

    handler->get();
    return handler;
}

cobraNetEvent*
cobraNetHandler::getEvent(int type)
{
    cobraNetEvent* event = NULL;
    cobraNetEventHandler* handler = getEventHandler(type);

    event = handler->eventGenesis();

    handler->put();
    return event;
}

bool
cobraNetHandler::removeEventHandler(int type)
{
    QWriteLocker locker(&m_eventLock);
    if (type <= cobraNetEventType)
        return false;

    if (!m_mihHandlers[type])
        return false;

    cobraNetEventHandler* handler = m_mihHandlers[type];
    m_mihHandlers.remove(type);

    handler->put();

    debug(LOW, "Handler (%s:%d) Removed", qPrintable(handler->name()), handler->type());

    return true;
}

bool
cobraNetHandler::isAuthorized(QString pass)
{
    return (pass == m_sSessionPwd || pass == m_sGuestPwd);
}

void
cobraNetHandler::setSessionPassword(QString pwd)
{
    m_sSessionPwd = pwd;
}

QString
cobraNetHandler::sessionPassword() const
{
    return m_sSessionPwd;
}

void
cobraNetHandler::setGuestPassword(QString pwd)
{
    m_sGuestPwd = pwd;
}

QString
cobraNetHandler::guestPassword() const
{
    return m_sGuestPwd;
}

int
cobraNetHandler::init()
{
    int ret = 0;
    for (int i = 0; i < m_iThreadCnt; i++) {
        QThread* thread = new QThread;
        cobraNetEventThread* worker = new cobraNetEventThread();

        QObject::connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanup()));

        worker->moveToThread(thread);
        thread->start();
        ret ++;

        m_cnetWorkers.push_back(worker);
        m_tThreads.push_back(thread);
    }
    return ret;
}

void
cobraNetHandler::cleanup()
{
    debug(CRITICAL, "Cleanup!\n");

    for (int x=0; x<m_tThreads.count(); x++) {
        if (!m_tThreads.at(x))
            continue;

        m_tThreads.at(x)->quit();
        m_tThreads.at(x)->wait();

        debug(CRITICAL, "Found Thread, Deleting.\n");
        m_tThreads.at(x)->deleteLater();
        m_cnetWorkers.at(x)->deleteLater();

        m_tThreads.replace(x, NULL);
        m_cnetWorkers.replace(x, NULL);
    }

    m_tThreads.clear();
    m_cnetWorkers.clear();
}

QSslCertificate cobraNetHandler::getCaCertificate() const
{
    QReadLocker locker(&m_certLock);
    return m_sslCaCertificate;
}

QSslCertificate cobraNetHandler::getLocalCertificate() const
{
    QReadLocker locker(&m_certLock);
    return m_sslLocalCertificate;
}

QSslKey cobraNetHandler::getPrivateKey() const
{
    QReadLocker locker(&m_certLock);
    return m_sslPrivateKey;
}

bool cobraNetHandler::loadCertificate(QString path, QSslCertificate& cert)
{
    QFile file(path);

    if(!file.exists()) {
        debug(ERROR(CRITICAL), "File '%s'' doesn't exists.\n", qPrintable(path));
        return false;
    }

    if(!file.open(QIODevice::ReadOnly)) {
        debug(ERROR(CRITICAL), "Cannot open file '%s'.\n", qPrintable(path));
        return false;
    }

    QByteArray data = file.readAll();

    QSslCertificate tmpcert(data, QSsl::Pem);

    /* For some reason, QSslCertificate won't load direction from a file via fromData... */
    cert = tmpcert;

    if(cert.isNull()) {
        debug(ERROR(CRITICAL), "The CA certificate has no content.\n");
        return false;
    }

    if(!cert.isValid()) {
        debug(ERROR(CRITICAL), "The CA certificate is NOT VALID!\n");
        return false;
    }
    return true;
}


bool cobraNetHandler::loadKey(QString path, QString password, QSslKey& key)
{
    QFile file(path);

    if(!file.exists()) {
        debug(ERROR(CRITICAL), "File '%s'' doesn't exists.\n", qPrintable(path));
        return false;
    }

    if(!file.open(QIODevice::ReadOnly)) {
        debug(ERROR(CRITICAL), "Cannot open filename %s.\n", qPrintable(path));
        return false;
    }

    QByteArray data = file.readAll();
    QByteArray pwd;
    pwd.append(password);

    QSslKey tempkey(data, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, pwd);

    /* Same as QSslCertificate -- won't work unless loading is done in a temp key. */
    key = tempkey;

    if(key.isNull()) {
        debug(ERROR(CRITICAL), "The CA certificate has no content.\n");
        return false;
    }

    return true;
}

bool cobraNetHandler::loadClientCertificates()
{
    QWriteLocker locker(&m_certLock);

    QString path = g_cobra_settings->value("ssl/ca").toString();

    if (path.isEmpty()) {
        debug(ERROR(CRITICAL), "Path to Certificate Unspecified!\n");
        return false;
    }

    if (!loadCertificate(path, m_sslCaCertificate)) {
        debug(ERROR(CRITICAL), "Failed to load Certificate!\n");
        return false;
    }

    return true;
}

bool cobraNetHandler::loadServerCertificates(QString passwd)
{
    QWriteLocker locker(&m_certLock);

    QString path = g_cobra_settings->value("ssl/ca").toString();

    if (path.isEmpty()) {
        debug(ERROR(CRITICAL), "Path to Certificate Unspecified!\n");
        return false;
    }

    if (!loadCertificate(path, m_sslCaCertificate)) {
        debug(ERROR(CRITICAL), "Failed to load Certificate!\n");
        return false;
    }

    path = g_cobra_settings->value("ssl/local_certificate").toString();

    if (path.isEmpty()) {
        debug(ERROR(CRITICAL), "Path to Local Certificate Unspecified!\n");
        return false;
    }

    if (!loadCertificate(path, m_sslLocalCertificate)) {
        debug(ERROR(CRITICAL), "Failed to load Local Certificate!\n");
        return false;
    }

    path = g_cobra_settings->value("ssl/private_key").toString();

    if (path.isEmpty()) {
        debug(ERROR(CRITICAL), "Path to Private Key Unspecified!\n");
        return false;
    }

    if (passwd.isEmpty()) {
        debug(ERROR(CRITICAL), "Loading PrivateKey Password from Config File!\n");
        path = g_cobra_settings->value("ssl/private_key_pwd").toString();
    }

    if (!loadKey(path, passwd, m_sslPrivateKey)) {
        debug(ERROR(CRITICAL), "Failed to load Private Key (Invalid password?)!\n");
        return false;
    }

    return true;
}

int
cobraNetHandler::optimalThread() const
{
    int minIdx = -1;
    int min = cobraNetEventCnxPerThread;
    for (int x=0; x<m_cnetWorkers.count(); x++)
        if (m_cnetWorkers[x] == NULL) {
            debug(ERROR(CRITICAL), "Failed to properly initialize threads!\n");
            return -1;
        } else {
            int tmp = m_cnetWorkers[x]->connections();
            debug(MED, "Worker %d has %d connections.\n", x, tmp);
            if (tmp < min)
                minIdx = x;
        }

    return minIdx;
}

int
cobraNetHandler::setAllowedErrors(QList<QSslError> err)
{
    QWriteLocker locker(&m_errorLock);
    m_lAllowedErrors = err;
    return m_lAllowedErrors.count();
}

void
cobraNetHandler::getAllowedErrors(QList<QSslError>& err)
{
    QReadLocker locker(&m_errorLock);
    err = m_lAllowedErrors;
}

bool
cobraNetHandler::setIdThread(cobraId id, int index)
{
    QWriteLocker locker(&m_cidLock);
    if (m_cIds.contains(id))
        return false;

    m_cIds[id] = index;
    return true;
}

bool
cobraNetHandler::listen(const QHostAddress& address, qint16 port) {
    m_idMine = cobraNetConnection::getNewId();
    bool res = QTcpServer::listen(address, port);
    return res;
}

bool
cobraNetHandler::connect(QString ip, int port, QString user, QString pass)
{
    int idx = optimalThread();
    if (idx < 0) {
        debug(ERROR(CRITICAL), "Failed to select optimal thread! (Are threads initialized?)\n");
        return false;
    }

    m_sUser = user;
    m_sPass = pass;

    debug(LOW, "Connecting: %lu\n", (unsigned long)QThread::currentThreadId());

    setIdThread(SERVER, idx);

    QMetaObject::invokeMethod(m_cnetWorkers[idx], "connect", Qt::QueuedConnection, Q_ARG(QString, ip), Q_ARG(int, port));
    return true;
}

void
cobraNetHandler::incomingConnection(int desc)
{
    int idx = optimalThread();
    if (idx < 0) {
        debug(ERROR(CRITICAL), "No threads found for incoming socket!\n");
        return;
    }

    debug(LOW, "Connection Request: %lu\n", (unsigned long)QThread::currentThreadId());

    cobraId id = cobraNetConnection::getNewId();
    setIdThread(id, idx);

    QMetaObject::invokeMethod(m_cnetWorkers[idx], "connectionRequest", Qt::QueuedConnection, Q_ARG(int, desc), Q_ARG(int, id));
}
