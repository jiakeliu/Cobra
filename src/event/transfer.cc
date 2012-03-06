#include "net.h"

cobraTransferEvent::cobraTransferEvent()
    :cobraNetEvent(cobraTransferEventType),
      m_iCommand(cobraTransferEvent::Request),
      m_uiUid(~0x0), m_iOffset(0)
{
}

cobraTransferEvent::cobraTransferEvent(cobraTransferEvent& state)
    :cobraNetEvent(state), m_iCommand(state.m_iCommand),
      m_uiUid(state.m_uiUid), m_iOffset(state.m_iOffset),
      m_baData(state.m_baData)
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
    stream << m_baData;
    return (bytes + sizeof(m_iOffset) + sizeof(m_uiUid) +
            sizeof(m_iCommand) + m_baData.length());
}

int
cobraTransferEvent::deserialize(QDataStream& stream)
{
    int bytes = cobraNetEvent::deserialize(stream);
    stream >> m_iCommand;
    stream >> m_uiUid;
    stream >> m_iOffset;
    stream >> m_baData;
    return (bytes + sizeof(m_iOffset) + sizeof(m_uiUid) +
            sizeof(m_iCommand) + m_baData.length());
}

cobraNetEvent*
cobraTransferEvent::duplicate()
{
    cobraTransferEvent* xfer = new cobraTransferEvent(*this);
    return xfer;
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
    bool ret = false;
    cobraNetHandler* handler = cobraNetHandler::instance();
    cobraTransferEvent* tevent = dynamic_cast<cobraTransferEvent*>(event);

    if (!tevent)
        return false;

    switch (tevent->command()) {
        case cobraTransferEvent::Chunk:
            ret = cobraTransferController::recieveChunk(tevent);
            tevent->put();
            return ret;

        case cobraTransferEvent::Request: {
            cobraTransferEvent* response = static_cast<cobraTransferEvent*>(tevent->duplicate());
            response->setDestination(tevent->source());
            response->setSource(SERVER);
            response->setCommand(cobraTransferEvent::Reject);

            int auth = handler->getIdAuthorization(event->source());
            if (auth & ParticipantAuth)
                response->setCommand(cobraTransferEvent::Accept);

            handler->sendEvent(response);
            response->put();
        }

        case cobraTransferEvent::Reject:
            QMessageBox::warning(NULL, "Transfer Rejected",
                                "The server rejected your file transfer request!\n"
                                "You probably do not have adequite permissions!");
    }

    tevent->put();
    return true;
}

bool
cobraTransferEventHandler::handleServerEvent(cobraNetEvent* event)
{
    (void)event;
    return false;
}

cobraNetEvent*
cobraTransferEventHandler::eventGenesis() const
{
    return new cobraTransferEvent();
}

