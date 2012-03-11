#include "debug.h"
#include "net.h"

cobraNetEventThread::cobraNetEventThread(QObject* parent, int cnx)
    :QObject(parent), m_ctcTransferController(cobraConcurrentTransfers),
      m_semAvailableConnections(cnx), m_iMaxConnections(cnx),
      m_uiLastMagic(0)
{
    /* Set the initial interval... */
    m_ctcTransferController.setInterval(10);

    if (!m_ctcTransferController.initialize(this))
        debug(ERROR(CRITICAL), "Failed to initialize the transfer controller!\n");

    m_baTransmitBuffer.reserve(4096);
}

cobraNetEventThread::~cobraNetEventThread()
{
    m_ctcTransferController.cleanup();
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

        debug(CRITICAL, "Sending Event: %d\n", event->type());
        int bytes = sendCnxEvent(m_cncConnections[idx], event);
        debug(ERROR(HIGH), "Sent: 0x%x bytes\n", bytes);
    }

    event->put();
    return true;
}

bool
cobraNetEventThread::sendFile(cobraTransferFile *file)
{
    if (!file)
        return false;

    cobraId dest = file->destination();

    debug(CRITICAL, "Destination: %d (isBroadcast: %s)\n", dest, (dest == BROADCAST)?"yes":"no");
    for (int idx=0; idx<m_cncConnections.count(); idx++) {
        if (!m_cncConnections[idx]) {
            debug(ERROR(CRITICAL), "Null Connection Found in Connection List! %d\n", idx);
            continue;
        }

        if (!m_cncConnections[idx]->is(dest))
            continue;

        return m_ctcTransferController.addTransfer(file);
    }

    return false;
}

bool
cobraNetEventThread::connectionRequest(int fd, int id)
{
    if (!m_semAvailableConnections.tryAcquire())
        return false;

    cobraNetConnection* cnx = new cobraNetConnection();
    if (!cnx)
        return false;

    cnx->setSocketDescriptor(fd);
    cnx->setId(id);
    debug(LOW, "Connecting: %d\n", id);

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
    /* Iterate through all connections and remove all that match ID (may be BROADCAST) */
    QVector<cobraNetConnection*>::iterator iter = m_cncConnections.begin();
    while (iter != m_cncConnections.end()) {

        if (!*iter) {
            debug(ERROR(CRITICAL), "Null connection encountered in connection list.\n");
            iter++;
            continue;
        }

        if (!(*iter)->is(id)) {
            debug(CRITICAL, "Skipping: %d\n", (*iter)->id());
            iter++;
            continue;
        }

        (*iter)->setConnected(false);

        cobraStateEvent event;
        event.setSource(SERVER);
        event.setResponse(true);
        event.setDestination((cobraId)id);
        event.setFlag(cobraStateEvent::Forced);
        event.setState(cobraStateEvent::ClosingState);

        debug(CRITICAL, "Sending Closing State Message to %d (cnx: %d)!\n", id, (*iter)->id());
        sendCnxEvent(*iter, &event);

        (*iter)->deleteLater();
        iter = m_cncConnections.erase(iter);
        m_semAvailableConnections.release();
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

    /* We only want ot send out a disconnect signal if we are still connected... */
    if (cnx->isConnected()) {
        if (cnx->is(SERVER)) {
            debug(LOW, "Disconnecting from client.\n");
            event->setSource(SERVER);
            event->setResponse(true);
            event->setDestination(cnx->id());
        } else {
            debug(LOW, "Disconnecting from server.\n");
            event->setSource(cnx->id());
            event->setResponse(false);
            event->setDestination(SERVER);
        }

        event->setState(cobraStateEvent::DisconnectedState);
        cobraSendEvent(event);

        cnx->setConnected(false);
    }
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

    cnx->setConnected(true);
    m_cncConnections.append(cnx);
}

void
cobraNetEventThread::clientReady()
{
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

    cnx->setConnected(true);
    m_cncConnections.append(cnx);

    event->setSource(SERVER);
    event->setResponse(true);
    event->setDestination(cobraMyId);
    event->setState(cobraStateEvent::ConnectingState);

    cobraSendEvent(event);
}

int
cobraNetEventThread::sendCnxEvent(cobraNetConnection *cnx, cobraNetEvent* event)
{
    QByteArray xmit;
    QDataStream stream(&xmit, QIODevice::ReadWrite);
    event->serialize(stream);

    uint32_t bytes = xmit.size();

    if (!bytes)
        return 0;

    debug(CRITICAL, "Wrote Magic: %ld\n", cnx->write(cobraStreamMagic));
    debug(CRITICAL, "Wrote Size: %ld\n", cnx->write((char*)&bytes, sizeof(bytes)));
    debug(CRITICAL, "Wrote Event: %ld\n", cnx->write(xmit));

    return bytes;
}

int
cobraNetEventThread::readyRead()
{
    cobraNetConnection* cnx = qobject_cast<cobraNetConnection*>(sender());
    if (!cnx)
        return 0;

    int type = 0;
    int bytes = 0;
    uint32_t size = 0;

    QByteArray events;
    int count = cnx->readEvents(events);
    QDataStream stream(&events, QIODevice::ReadWrite);

    debug(ERROR(LOW), "Ready Read: %d\n", count);
    for (int x=0; x<count; x++) {

        stream.skipRawData(cobraStreamMagic.count() + sizeof(size));
        /* if we've made it here, then we should have a valid event, lets check. */

        stream >> type;
        bytes += sizeof(type);

        if (!validEvent(type)) {
            debug(ERROR(CRITICAL), "Invalid event after a Magic... what are the odds...\n");
            continue;
        }

        cobraNetEvent* event = cobraNetHandler::instance()->getEvent(type);
        if (!event) {
            debug(ERROR(CRITICAL), "Invalid type (%d) sent?\n", type);
            continue;
        }

        bytes += event->deserialize(stream);
        debug(ERROR(HIGH), "Bytes read from incoming event: %d\n", bytes);

        if (cnx->id() != event->source())
            debug(ERROR(CRITICAL), "Incoming event failed source coherenecy check! "
                  "(Cnx: %d; Source: %d)\n", cnx->id(), event->source());

        event->setSource(cnx->id());

        if (event->type() == cobraTransferEventType) {
            cobraTransferEvent* xevent = static_cast<cobraTransferEvent*>(event);
            if (m_ctcTransferController.interceptEvent(xevent)) {
                xevent->put();
                continue;
            }
        }

        cobraSendEvent(event);
    }

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

    event->setSource(cnx->id());
    event->setResponse(false);
    event->setDestination(SERVER);
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
