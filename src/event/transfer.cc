#include "net.h"
#include "debug.h"

cobraTransferEvent::cobraTransferEvent()
    :cobraNetEvent(cobraTransferEventType),
      m_iCommand(cobraTransferEvent::Request),
      m_uiUid(~0x0), m_iOffset(0), m_iSize(0)
{
}

cobraTransferEvent::cobraTransferEvent(cobraTransferEvent& state)
    :cobraNetEvent(state), m_iCommand(state.m_iCommand),
      m_uiUid(state.m_uiUid), m_iOffset(state.m_iOffset), m_iSize(state.m_iSize),
      m_baHash(state.m_baHash), m_baData(state.m_baData), m_sExtension(state.m_sExtension)
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
    stream << m_sExtension;

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
    stream >> m_sExtension;

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

QString cobraTransferEvent::extension()
{
    return m_sExtension;
}

void cobraTransferEvent::setExtension(QString fileName)
{
    int index = fileName.lastIndexOf(".");
    if (index < 0)
        m_sExtension = fileName;
    else
        m_sExtension = fileName.remove(0, index+1);

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
cobraTransferEvent::setSize(qint64 size)
{
    m_iSize = size;
}

qint64
cobraTransferEvent::size() const
{
    return m_iSize;
}

void
cobraTransferEvent::setData(QByteArray& data)
{
    m_baData = data;
}

void
cobraTransferEvent::clearData()
{
    m_baData.clear();
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
    debug(MED, "Transfer Handler!\n");
    bool ret = false;
    cobraNetHandler* handler = cobraNetHandler::instance();
    cobraTransferEvent* tevent = dynamic_cast<cobraTransferEvent*>(event);

    if (!tevent)
        return false;

    switch (tevent->command()) {

    case cobraTransferEvent::Chunk: {
        debug(LOW, "Transfer Chunk!\n");

        int cmp = cobraTransferController::recieveChunk(tevent);

        switch (cmp) {
        case cobraTransferFile::TransferComplete: {
            cobraTransferEvent* xevent = static_cast<cobraTransferEvent*>(event->duplicate());

            xevent->setCommand(cobraTransferEvent::Complete);
            xevent->setSource(event->destination());
            xevent->setDestination(event->source());
            xevent->setResponse((event->source()==SERVER));
            xevent->setOffset(0);
            xevent->setSize(0);
            xevent->clearData();

            debug(HIGH, "Transfer Complete: %d\n", xevent->uid());

            handler->sendEvent(xevent);
            break;
        }
        case cobraTransferFile::TransferError: {
            debug(ERROR(CRITICAL), "Error while recieving chunk for file: %d with hash %s\n",
                  tevent->uid(), tevent->hash().toHex().data());
            break;
        }
        }

        tevent->put();
        return ret;
    }

    case cobraTransferEvent::Request: {
        debug(LOW, "Transfer Request!\n");

        cobraTransferEvent* response = static_cast<cobraTransferEvent*>(tevent->duplicate());
        response->setDestination(tevent->source());
        response->setSource(tevent->destination());
        response->setCommand(cobraTransferEvent::Reject);
        response->setResponse(true);

        int auth = handler->getIdAuthorization(event->source());
        if (auth & ParticipantAuth) {
            QString filePath = g_cobra_settings->value("storage_dir").toString();


            QString extension = tevent->extension();
            QByteArray hash = tevent->hash();
            filePath += "/" + hash.toHex() + "." + extension;

            cobraTransferFile* file = new cobraTransferFile(filePath);

            file->setUid(tevent->uid());
            file->setExpectedHash(hash);
            file->setDestination(tevent->destination());
            file->setSource(tevent->source());
            file->setSending(false);

            if (!addTransfer(file))
                debug(ERROR(CRITICAL), "Invalid file!\n");
            else
                response->setCommand(cobraTransferEvent::Accept);
        }

        handler->sendEvent(response);
        response->put();
        break;
    }

    case cobraTransferEvent::Accept: {
        debug(LOW, "Transfer Accept!\n");

        cobraTransferFile* file = cobraTransferController::getPendingTransfer(tevent->uid(), tevent->hash());
        if (!file) {
            debug(ERROR(CRITICAL), "Failed to find valid file!\n");
            break;
        }

        file->setDestination(event->source());
        file->setSource(event->destination());
        handler->sendFile(file);
        break;
    }

    case cobraTransferEvent::Reject:
        debug(LOW, "Transfer Rejected!\n");
        QMessageBox::warning(NULL, "Transfer Rejected",
                             "The server rejected your file transfer request!\n"
                             "You probably do not have adequite permissions!");
        break;


    case cobraTransferEvent::Resend:
        //fall-through
    case cobraTransferEvent::Complete:
        debug(ERROR(HIGH), "This should be intercepted!\n");
        break;
    }

    tevent->put();
    return true;
}

bool
cobraTransferEventHandler::handleServerEvent(cobraNetEvent* event)
{
    debug(ERROR(CRITICAL), "Somehow we called into the cobraTransferEvent::handleServerEvent handler...\n");
    exit(1);
    (void)event;
    return false;
}

cobraNetEvent*
cobraTransferEventHandler::eventGenesis() const
{
    return new cobraTransferEvent();
}

