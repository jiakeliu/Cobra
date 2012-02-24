#include "debug.h"
#include "net.h"

cobraNetEventThread::cobraNetEventThread(QObject* parent, int cnx)
    :QObject(parent), m_semAvailableConnections(cnx), m_iMaxConnections(cnx)
{
}

cobraNetEventThread::~cobraNetEventThread()
{
}

bool
cobraNetEventThread::sendEvent(cobraNetEvent *event)
{
    if (!event)
        return false;

    debug(CRITICAL, "Sending event!\n");
    cobraId dest = event->destination();

    debug(CRITICAL, "Destination: %d (isBroadcast: %s)\n", dest, (dest == BROADCAST)?"yes":"no");
    for (int idx=0; idx<m_cncConnections.count(); idx++) {
        if (m_cncConnections[idx])
            debug(CRITICAL, "Found Connection! %d\n", idx);
        if (m_cncConnections[idx] && m_cncConnections[idx]->is(dest)) {
            QDataStream stream(m_cncConnections[idx]);

            debug(CRITICAL, "Sending Data!\n");
            stream << cobraStreamMagic;
            event->serialize(stream);
        }
    }

    return false;
}

bool
cobraNetEventThread::connectionRequest(int fd, int id)
{
    debug(LOW, "Connect: %d\n", fd);
    debug(LOW, "Connect: %lu\n", (unsigned long)QThread::currentThreadId());
    if (!m_semAvailableConnections.tryAcquire())
        return false;

    cobraNetConnection* cnx = new cobraNetConnection();
    if (!cnx)
        return false;

    cnx->setSocketDescriptor(fd);
    cnx->setId(id);

    QObject::connect(cnx, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sockError(QAbstractSocket::SocketError)));
    QObject::connect(cnx, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));

    QObject::connect(cnx, SIGNAL(disconnected()), this, SLOT(disconnect()));
    QObject::connect(cnx, SIGNAL(readyRead()), this, SLOT(readyRead()));
    QObject::connect(cnx, SIGNAL(encrypted()), this, SLOT(serverReady()));

    cnx->addCaCertificate(cobraNetHandler::instance()->getCaCertificate());
    cnx->setPrivateKey(cobraNetHandler::instance()->getPrivateKey());
    cnx->setLocalCertificate(cobraNetHandler::instance()->getLocalCertificate());

    cnx->startServerEncryption();
    return true;
}

bool
cobraNetEventThread::connect(QString ip, int port)
{
    debug(LOW, "Connect: %s:%d\n", qPrintable(ip), port);
    debug(LOW, "Connect: %lu\n", (unsigned long)QThread::currentThreadId());

    if (!m_semAvailableConnections.tryAcquire())
        return false;

    cobraNetConnection* cnx = new cobraNetConnection(this);
    if (!cnx)
        return false;

    cnx->moveToThread(this->thread());

    QObject::connect(cnx, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    QObject::connect(cnx, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sockError(QAbstractSocket::SocketError)));

    QObject::connect(cnx, SIGNAL(disconnected()), this, SLOT(disconnect()));
    QObject::connect(cnx, SIGNAL(readyRead()), this, SLOT(readyRead()));
    QObject::connect(cnx, SIGNAL(encrypted()), this, SLOT(clientReady()));

    cnx->addCaCertificate(cobraNetHandler::instance()->getCaCertificate());
    cnx->connectToHostEncrypted(ip, port);

    return true;
}

void
cobraNetEventThread::disconnect()
{
    debug(LOW, "Disconnect: %lu\n", (unsigned long)QThread::currentThreadId());
    m_semAvailableConnections.release();

   //add disconnect update info
   //set the connected state in state.cc

    // model the disconnect off this
    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());
    cobraStateEvent* event = new cobraStateEvent();

    if (!cnx) {
        debug(ERROR(CRITICAL), "WTF: No Connection Associated with the ClientReady signal?");
        return;
    }

    cnx->setId(SERVER);
    m_cncConnections.append(cnx);

//make set connect a ::friend
    event->setSource(SERVER);
    event->setResponse(true);
    event->setDestination(NO_ID);
    event->setState(QAbstractSocket::ClosingState);

    cobraSendEvent(event);
}

void
cobraNetEventThread::serverReady()
{
    debug(LOW, "Server Ready: %lu\n", (unsigned long)QThread::currentThreadId());

    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());

    if (!cnx) {
        debug(ERROR(CRITICAL), "WTF: No Connection Associated with the ClientReady signal?");
        return;
    }

    m_cncConnections.append(cnx);
}

void
cobraNetEventThread::clientReady()
{
    debug(LOW, "Client Ready: %lu\n", (unsigned long)QThread::currentThreadId());
// model the disconnect off this
    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());
    cobraStateEvent* event = new cobraStateEvent();

    if (!cnx) {
        debug(ERROR(CRITICAL), "WTF: No Connection Associated with the ClientReady signal?");
        return;
    }

    cnx->setId(SERVER);
    m_cncConnections.append(cnx);

    event->setSource(SERVER);
    event->setResponse(true);
    event->setDestination(NO_ID);
    event->setState(QAbstractSocket::ConnectingState);

    cobraSendEvent(event);
}

int
cobraNetEventThread::readyRead()
{
    debug(LOW, "Ready Read\n");
    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());
    if (!cnx)
        return 0;

    QDataStream stream(cnx);

    int magic = 0;
    int type = 0;

    while (magic != cobraStreamMagic)
    {
        stream >> magic;
        debug(CRITICAL, "Checking Magic!\n");
        if (stream.atEnd())
            return 0;
    }

    stream >> type;

    cobraNetEvent* event = cobraNetHandler::instance()->getEvent(type);
    if (!event)
        return 0;

    int bytes = event->deserialize(stream);
    debug(HIGH, "Bytes read from incoming event: %d\n", bytes);

    if (event->source() == NO_ID) {
        event->setSource(cnx->id());
    }

    cobraSendEvent(event);
    return bytes;
}

int
cobraNetEventThread::sockError(QAbstractSocket::SocketError error)
{
    QIODevice* device = qobject_cast<QIODevice*>(sender());
    if (!device) {
        debug(ERROR(CRITICAL), "Socket Error: %d (Not Sent By Device)\n", error);
        return -1;
    }
    debug(ERROR(CRITICAL), "Socket Error: %d\n", error);
// added disconnect
    disconnect();
    return error;
}

int
cobraNetEventThread::sslErrors(QList<QSslError> errors)
{
    debug(ERROR(CRITICAL), "OMG SSL ERRORS!!!\n");
    for (int x=0; x<errors.count(); x++) {
        debug(ERROR(CRITICAL), "SSL Error (%d): %s\n", errors.at(x).error(), qPrintable(errors.at(x).errorString()));
    }
    return errors.count();
}
