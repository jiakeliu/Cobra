#include "net.h"
#include "debug.h"

cobraTransferEvent::cobraTransferEvent()
    :cobraNetEvent(cobraTransferEventType),
      m_iCommand(cobraTransferEvent::Request),
      m_uiUid(~0x0), m_iOffset(0)
{
}

cobraTransferEvent::cobraTransferEvent(cobraTransferEvent& state)
    :cobraNetEvent(state), m_iCommand(state.m_iCommand),
      m_uiUid(state.m_uiUid), m_iOffset(state.m_iOffset), m_iSize(state.m_iSize),
      m_baHash(state.m_baHash), m_baData(state.m_baData)
{
}

cobraTransferEvent::~cobraTransferEvent()
{
}

int
cobraTransferEvent::serialize(QDataStream& stream)
{
    int bytes = cobraNetEvent::serialize(stream);
    stream << m_iCommand;
    stream << m_uiUid;
    stream << m_iOffset;
    stream << m_iSize;
    stream << m_baHash;
    stream << m_baData;
    return (bytes + sizeof(m_iOffset) + sizeof(m_uiUid) +
            sizeof(m_iCommand) + m_baData.length() +
            sizeof(m_iSize) + sizeof(m_iOffset) + m_baHash.length());
}

int
cobraTransferEvent::deserialize(QDataStream& stream)
{
    int bytes = cobraNetEvent::deserialize(stream);
    stream >> m_iCommand;
    stream >> m_uiUid;
    stream >> m_iOffset;
    stream >> m_iSize;
    stream >> m_baHash;
    stream >> m_baData;
    return (bytes + sizeof(m_iOffset) + sizeof(m_uiUid) +
            sizeof(m_iCommand) + m_baData.length() +
            sizeof(m_iSize) + sizeof(m_iOffset) + m_baHash.length());
}

cobraNetEvent*
cobraTransferEvent::duplicate()
{
    cobraTransferEvent* xfer = new cobraTransferEvent(*this);
    return xfer;
}

bool
cobraTransferEvent::fromFile(cobraTransferFile* file)
{
    if (!file)
        return false;

    if (!file->exists())
        return false;

    m_uiUid = file->uid();
    m_baHash = file->hash();
    m_iOffset = 0;
    m_iSize = file->size();

    setDestination(file->destination());
    setSource(file->source());

    return true;
}

void
cobraTransferEvent::setCommand(int cmd)
{
    m_iCommand = cmd;
}

int
cobraTransferEvent::command() const
{
    return m_iCommand;
}

void
cobraTransferEvent::setUid(uint32_t uid)
{
    m_uiUid = uid;
}

uint32_t
cobraTransferEvent::uid() const
{
    return m_uiUid;
}

void
cobraTransferEvent::setOffset(qint64 offset)
{
    m_iOffset = offset;
}

qint64
cobraTransferEvent::offset() const
{
    return m_iOffset;
}

void
cobraTransferEvent::setData(QByteArray& data)
{
    m_baData = data;
}

const QByteArray&
cobraTransferEvent::data() const
{
    return m_baData;
}

void
cobraTransferEvent::setHash(QByteArray& hash)
{
    m_baHash = hash;
}

const QByteArray&
cobraTransferEvent::hash() const
{
    return m_baHash;
}

cobraTransferEventHandler::cobraTransferEventHandler()
    :cobraNetEventHandler("Transfer", cobraTransferEventType),
      cobraTransferController(cobraConcurrentTransfers)
{
    initialize();
}

cobraTransferEventHandler::cobraTransferEventHandler(cobraTransferEventHandler& event)
    :cobraNetEventHandler(event), cobraTransferController(cobraConcurrentTransfers)
{
    initialize();
}

cobraTransferEventHandler::~cobraTransferEventHandler()
{
    cleanup();
}

bool
cobraTransferEventHandler::handleEvent(cobraNetEvent* event)
{
    debug(ERROR(CRITICAL), "Transfer Handler!\n");
    bool ret = false;
    cobraNetHandler* handler = cobraNetHandler::instance();
    cobraTransferEvent* tevent = dynamic_cast<cobraTransferEvent*>(event);

    if (!tevent)
        return false;

    switch (tevent->command()) {
    case cobraTransferEvent::Chunk: {
        debug(ERROR(CRITICAL), "Transfer Chunk!\n");
        int cmp = cobraTransferController::recieveChunk(tevent);
        if (cmp == cobraTransferController::TransferComplete) {
            cobraTransferEvent* xevent = static_cast<cobraTransferEvent*>(event->duplicate());
            xevent->setSource(event->destination());
            xevent->setDestination(event->source());
            xevent->setCommand(cobraTransferEvent::Complete);
            handler->sendEvent(xevent);
        }
        tevent->put();
        return ret;
    }

    case cobraTransferEvent::Request: {
        debug(ERROR(CRITICAL), "Transfer Request!\n");
        cobraTransferEvent* response = static_cast<cobraTransferEvent*>(tevent->duplicate());
        response->setDestination(tevent->source());
        response->setSource(tevent->destination());
        response->setCommand(cobraTransferEvent::Reject);
        response->setResponse(true);

        int auth = handler->getIdAuthorization(event->source());
        if (auth & ParticipantAuth)
            response->setCommand(cobraTransferEvent::Accept);

        handler->sendEvent(response);
        response->put();
        break;
    }

    case cobraTransferEvent::Accept: {
        debug(ERROR(CRITICAL), "Transfer Accept!\n");
        cobraTransferFile* file = cobraTransferController::getPendingTransfer(tevent->uid(), tevent->hash());
        if (!file)
            break;

        file->setDestination(event->source());
        file->setSource(event->destination());
        handler->sendFile(file);
        break;
    }

    case cobraTransferEvent::Reject:
        debug(ERROR(CRITICAL), "Transfer Rejected!\n");
        QMessageBox::warning(NULL, "Transfer Rejected",
                             "The server rejected your file transfer request!\n"
                             "You probably do not have adequite permissions!");
        break;


    case cobraTransferEvent::Resend:
    case cobraTransferEvent::Complete:
        debug(ERROR(CRITICAL), "This should be intercepted!\n");
        break;
    }

    tevent->put();
    return true;
}

bool
cobraTransferEventHandler::handleServerEvent(cobraNetEvent* event)
{
    (void)event;
    fprintf(stderr, "Somehow we called into the cobraTransferEvent::handleServerEvent handler...\n");
    exit(1);
    return false;
}

cobraNetEvent*
cobraTransferEventHandler::eventGenesis() const
{
    return new cobraTransferEvent();
}

