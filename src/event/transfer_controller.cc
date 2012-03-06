#include "event.h"
#include "debug.h"

cobraTransferStatistics::cobraTransferStatistics()
{}

cobraTransferStatistics::~cobraTransferStatistics()
{}

cobraTransferStatistics* cobraTransferStatistics::instance()
{
    static cobraTransferStatistics* stats = NULL;
    if (!stats)
        stats = new cobraTransferStatistics;
    return stats;
}

void
cobraTransferStatistics::updatePath(uint32_t uid, QString path)
{
    QWriteLocker lock(&m_rwlStatLock);
    m_mumMetrics[uid].path = path;
}

void
cobraTransferStatistics::updateSize(uint32_t uid, unsigned long size)
{
    QWriteLocker lock(&m_rwlStatLock);
    m_mumMetrics[uid].size = size;
}

void
cobraTransferStatistics::updateCurrent(uint32_t uid, unsigned long current)
{
    QWriteLocker lock(&m_rwlStatLock);
    m_mumMetrics[uid].current = current;
}

void
cobraTransferStatistics::updateTime(uint32_t uid, unsigned long time)
{
    QWriteLocker lock(&m_rwlStatLock);
    m_mumMetrics[uid].time = time;
}

void
cobraTransferStatistics::updateMap(cobraStatMap& map) const
{
    QReadLocker lock(&m_rwlStatLock);
    map = m_mumMetrics;
}

cobraTransferFile::cobraTransferFile(QString& path)
    :QFile(path)
{}

cobraTransferFile::~cobraTransferFile()
{}

uint32_t
cobraTransferFile::uid() const
{
    return m_uiUid;
}

void
cobraTransferFile::setUid(uint32_t uid)
{
    m_uiUid = uid;
}

bool
cobraTransferFile::isTransfer(uint32_t uid) const
{
    return (uid == m_uiUid);
}

bool
cobraTransferFile::isActive() const
{
    return m_bActive;
}

void
cobraTransferFile::activate(bool enable)
{
    m_bActive = enable;
}

bool
cobraTransferFile::isSending() const
{
    return m_bSending;
}

void
cobraTransferFile::setSending(bool sending)
{
    m_bSending = sending;
}

void
cobraTransferFile::setDestination(cobraId dest)
{
    m_idDestination = dest;
}

void
cobraTransferFile::setSource(cobraId src)
{
    m_idSource = src;
}

cobraId
cobraTransferFile::destination() const
{
    return m_idDestination;
}

cobraId
cobraTransferFile::source() const
{
    return m_idSource;
}

bool
cobraTransferFile::sendChunk(cobraNetEventThread* thread, qint64 chunk, qint64 offset)
{
    /**
     * Here, we want to validate the file has been made active,
     * if so, we want to store the next CHUNK bytes into a QByteArray,
     * create a transfer event, store the QByteArray, and then send it
     * out.
     *
     * send:
     *  event->setSource(m_idSource);
     *  event->setDestination(m_idDestination);
     *  thread->sendEvent(event);
     */

    if (offset != CURRENT_OFFSET)
        seek(offset);

    QByteArray chunkArray;

    if( isActive() == true) {      
        cobraTransferFile::setSource(m_idSource);
        cobraTransferFile::setDestination(m_idDestination);
        //cobraNetEventThread::sendEvent(chunkArray);
    }
    else
        return false;

}

bool
cobraTransferFile::recieveChunk(cobraTransferEvent* event)
{
    if (!event || m_bSending)
        return false;

    return true;
}

cobraTransferController::cobraTransferController(int concurrent, QObject* parent)
    :QObject(parent), m_iNextTransfer(-1),
      m_iConcurrentTransfers(concurrent), m_iChunkSize(1024),
      m_netParent(NULL)
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
cobraTransferController::setChunkSize(qint64 chunk)
{
    m_iChunkSize = chunk;
}

qint64
cobraTransferController::chunkSize() const
{
    return m_iChunkSize;
}

int
cobraTransferController::interval() const
{
    return m_tTransferTimer.interval();
}

void
cobraTransferController::setInterval(int interval)
{
    m_tTransferTimer.setInterval(interval);
}

bool
cobraTransferController::processTrigger()
{
    /** Send the next file chunk and increment.
     * If we reach the end of the file, then
     * we need to remove that file from the queue,
     * activate and activate the first inactive one
     * in the list.  If we did not reach the end,
     * iterate our m_iNextTransfer, and wrap around
     * if m_iNextTransfer == m_iConcurrentTransfers.
     */

    if (m_iNextTransfer < 0) {
        //debug(CRITICAL, "Process Trigger: Nothing to process!\n");
        return true;
    }

    return false;
}

bool
cobraTransferController::initialize(cobraNetEventThread* parent)
{
    m_netParent = parent;

    QObject::connect(&m_tTransferTimer, SIGNAL(timeout()), this, SLOT(processTrigger()));

    /* start timer, if we are the transmitter... */
    if (m_netParent) {
        m_tTransferTimer.start();
    }

    return true;
}

void
cobraTransferController::cleanup()
{
    /* iterate through all open files and close them... */
    m_tTransferTimer.stop();
}

bool
cobraTransferController::handleTransfer(cobraTransferEvent* event)
{
    /** This function should iterate through the cobraTransferFile list
      * and call the handleTransfer on each one (only the one that matches
      * will return true);
      * Before actualy passing it on though, the command should be checked,
      * to properly handle the generation of any new transfers.
      */

    /** The way I see the transfer init working is that when a file is to be
      * sent, the cobraTransferEvent containing relevent information is sent
      * immediately (so, if you wanna send 5 files, but you can only send one
      * at a time, you send 5 transfer events describing the files, and then
      * start sending actual data one file at a time). All the transfer stuff
      * will be triggered by the timer (which should be the 'processTrigger')
      * This function will intercept the initial transfer request, generate an
      * entry in the pending list.  Once a accept request is recieved, the
      * cobraTransferFile will be moved to the active list.
      */
    return false;
}
