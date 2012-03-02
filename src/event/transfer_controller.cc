#include "event.h"

cobraFileTransfer::cobraFileTransfer(QString& path)
    :QFile(path)
{}

cobraFileTransfer::~cobraFileTransfer()
{}

uint32_t
cobraFileTransfer::uid() const
{
    return m_uiUid;
}

void
cobraFileTransfer::setUid(uint32_t uid)
{
    m_uiUid = uid;
}

bool
cobraFileTransfer::isTransfer(uint32_t uid) const
{
    return (uid == m_uiUid);
}

bool
cobraFileTransfer::isActive() const
{
    return m_bActive;
}

void
cobraFileTransfer::activate(bool enable)
{
    m_bActive = enable;
}

void
cobraFileTransfer::setDestination(cobraId dest)
{
    m_idDestination = dest;
}

void
cobraFileTransfer::setSource(cobraId src)
{
    m_idSource = src;
}

cobraId
cobraFileTransfer::destination() const
{
    return m_idDestination;
}

cobraId
cobraFileTransfer::source() const
{
    return m_idSource;
}

bool
cobraFileTransfer::sendChunk(int chunk)
{
    return false;
}

bool
cobraFileTransfer::handleTransfer(cobraTransferEvent* event)
{
    return false;
}

cobraTransferController::cobraTransferController(int concurrent, QObject* parent)
    :QObject(parent), m_iConcurrentTransfers(concurrent)
{}

cobraTransferController::~cobraTransferController()
{}

void
cobraTransferController::setTransferCount(int con)
{
    m_iConcurrentTransfers = con;
}

int
cobraTransferController::transferCount() const
{
    return m_iConcurrentTransfers;
}

void
cobraTransferController::setChunkSize(int chunk)
{
    m_iChunkSize = chunk;
}

int
cobraTransferController::chunkSize() const
{
    return m_iChunkSize;
}

bool
cobraTransferController::handleTransfer(cobraTransferEvent* event)
{
    return false;
}
