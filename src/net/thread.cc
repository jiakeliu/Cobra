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

    cobraId dest = event->destination();

    debug(CRITICAL, "Destination: %d (isBroadcast: %s)\n", dest, (dest == BROADCAST)?"yes":"no");
    for (int idx=0; idx<m_cncConnections.count(); idx++) {
        if (!m_cncConnections[idx]) {
            debug(ERROR(CRITICAL), "Null Connection Found in Connection List! %d\n", idx);
            continue;
        }

        if (!m_cncConnections[idx]->is(dest))
            continue;

        QDataStream stream(m_cncConnections[idx]);

        debug(CRITICAL, "Sending Data!\n");
        stream << cobraStreamMagic;
        event->serialize(stream);
    }

    return false;
}

bool
cobraNetEventThread::connectionRequest(int fd, int id)
{
    debug(LOW, "Connect: %d\n", fd);
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

    if (!m_semAvailableConnections.tryAcquire())
        return false;

    cobraNetConnection* cnx = new cobraNetConnection(this);
    if (!cnx)
        return false;

    cnx->moveToThread(this->thread());
    cnx->setId(SERVER);

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
cobraNetEventThread::removeConnection(int id)
{
    QVector<cobraNetConnection*>::iterator iter;
    for (iter=m_cncConnections.begin(); iter<m_cncConnections.end(); iter++) {
        if (!*iter) {
            debug(ERROR(CRITICAL), "Null connection encountered in connection list.\n");
            continue;
        }

        if (!(*iter)->is(id))
            continue;

        cobraStateEvent event;

        event.setSource(SERVER);
        event.setResponse(true);
        event.setDestination((cobraId)id);
        event.setFlag(cobraStateEvent::Forced);
        event.setState(cobraStateEvent::ClosingState);

        { /* Make our QDataStream object is destroyed before deleteLater call */
            debug(CRITICAL, "Sending Closing State Message to %d (cnx: %d)!\n", id, (*iter)->id());
            QDataStream stream(*iter);
            stream << cobraStreamMagic;
            event.serialize(stream);
        }

        m_cncConnections.erase(iter);
        m_semAvailableConnections.release();
        (*iter)->deleteLater();
    }
}

void
cobraNetEventThread::disconnect()
{
    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());
    cobraStateEvent* event = new cobraStateEvent();

    if (!cnx) {
        debug(ERROR(CRITICAL), "WTF: No Connection Associated with the ClientReady signal?");
        return;
    }

    if (cnx->is(SERVER)) {
        debug(LOW, "Disconnect (As a client).\n");
        event->setSource(SERVER);
        event->setResponse(true);
        event->setDestination(BROADCAST);
    } else {
        debug(LOW, "Disconnect (As the server).\n");
        event->setSource(cnx->id());
        event->setResponse(false);
        event->setDestination(SERVER);
    }

    event->setState(cobraStateEvent::DisconnectedState);
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

    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());
    cobraStateEvent* event = new cobraStateEvent();

    if (!cnx) {
        debug(ERROR(CRITICAL), "We've already cleaned up this connection, nothing to see here...\n");
        return;
    }

    if (cnx->id() != SERVER) {
        debug(ERROR(HIGH), "Uh, we connected to a server without setting its ID?\nThis should never happen ^_^.\n");
        exit(1);
    }
    m_cncConnections.append(cnx);

    event->setSource(SERVER);
    event->setResponse(true);
    event->setDestination(BROADCAST);
    event->setState(cobraStateEvent::ConnectingState);

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

    if (cnx->id() != event->source())
        debug(ERROR(CRITICAL), "Incoming event failed source coherenecy check! (Cnx: %d; Source: %d)\n",
              cnx->id(), event->source());

    event->setSource(cnx->id());

    cobraSendEvent(event);
    return bytes;
}

void
cobraNetEventThread::connectionRefused()
{
    debug(LOW, "Disconnect: %lu\n", (unsigned long)QThread::currentThreadId());
    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());
    cobraStateEvent* event = new cobraStateEvent();

    if (!cnx) {
        debug(ERROR(CRITICAL), "WTF: No Connection Associated with the ClientReady signal?");
        return;
    }

    if (cnx->is(SERVER)) {
        event->setSource(SERVER);
        event->setResponse(true);
        event->setDestination(BROADCAST);
    } else {
        event->setSource(cnx->id());
        event->setResponse(false);
        event->setDestination(SERVER);
    }

    event->setState(cobraStateEvent::ConnectionRefused);
    cobraSendEvent(event);
}

int
cobraNetEventThread::sockError(QAbstractSocket::SocketError error)
{
    QIODevice* device = qobject_cast<QIODevice*>(sender());
    if (!device) {
        debug(ERROR(CRITICAL), "Socket Error: %d (Not Sent By Device)\n", error);
        return -1;
    }
    debug(ERROR(CRITICAL), "Socket Error (%d): %s\n", error, qPrintable(device->errorString()));

    switch(error) {
        /* Only disconnect if this error is a host disconnect error... */
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::ConnectionRefusedError:
            connectionRefused();
            break;
        default:
            disconnect();
    }
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
