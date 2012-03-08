#include "net.h"
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
    if (enable)
        open(ReadOnly);
    else
        close();
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
    if (sending)
        m_baHash = hash();
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
cobraTransferFile::is(uint32_t id) const
{
    return (id == m_uiUid);
}

bool
cobraTransferFile::sendChunk(cobraNetEventThread* thread, qint64 chunk)
{
    /**
     * Here, we want to validate the file has been made active,
     * if so, we want to store the next CHUNK bytes into a QByteArray,
     * create a transfer event, store the QByteArray, and then send it
     * out.
     */

    /* this should the next chunk that needs to be sent.....
     * if the chunk is smaller than the chunk,
     * just send the size requested, if greater, break it up into chunk sizes...
     * as chunks are sent, this should be updated to keep track of data which
     * still needs to be sent
     */
    QByteArray data = read(chunk);

    if(isActive()) {
        cobraTransferEvent* event = new cobraTransferEvent();
        /*
          --- set source and destination
          */
        event->setData(data);
        event->setHash(m_baHash);

        return thread->sendEvent(event);
    }


    return false;
}

bool
cobraTransferFile::recieveChunk(cobraTransferEvent* event)
{
    if (!event || m_bSending)
        return false;

    return true;
}

bool
cobraTransferFile::transferComplete()
{
    // TODO: This needs to close the file and do anything else necessary
    return false;
}

bool
cobraTransferFile::resendChunk(qint64 chunk, qint64 offset)
{
    // TODO: This needs to update the chunk list to allow for this chunk to be hte next
    // one sent on the next sendChunk call..
    return false;
}

QByteArray
cobraTransferFile::hash()
{
    if (m_bSending) {
        if (m_baHash.isEmpty() || m_baHash.isNull())
           m_baHash = QCryptographicHash::hash(readAll(), QCryptographicHash::Md5);
        return m_baHash;
    }
    return m_baHash;
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
    bool ret = false;

    if (m_iNextTransfer < 0) {
        //debug(CRITICAL, "Process Trigger: Nothing to process!\n");
        return true;
    }

    m_iNextTransfer %= (m_vcftTransfers.size()<m_iConcurrentTransfers)?
                                m_vcftTransfers.size():
                                m_iConcurrentTransfers;

    cobraTransferFile* file = NULL;

    do {
        file = m_vcftTransfers.at(m_iNextTransfer);

        if (!file) {
            m_vcftTransfers.remove(m_iNextTransfer);
            continue;
        }
    } while (!file);

    if (!file->isActive())
        file->activate(true);

    ret = file->sendChunk(m_netParent, m_iChunkSize);

    m_iNextTransfer++;
    return ret;
}

bool
cobraTransferController::initialize(cobraNetEventThread* parent)
{
    m_netParent = parent;

    QObject::connect(&m_tTransferTimer, SIGNAL(timeout()), this, SLOT(processTrigger()));

    /* start timer, if we are the transmitter... */
    if (m_netParent)
        m_tTransferTimer.start();

    return true;
}

void
cobraTransferController::cleanup()
{
    /* iterate through all open files and close them... */
    for (int x=0; x<m_vcftTransfers.size(); x++) {
        cobraTransferFile* file = m_vcftTransfers[x];

        file->activate(false);
        delete file;
    }

    m_vcftTransfers.clear();
    m_tTransferTimer.stop();
}

bool
cobraTransferController::interceptEvent(cobraTransferEvent *event)
{
    if (!event)
        return false;

    switch (event->command()) {
        case cobraTransferEvent::Resend:
            return true;

        case cobraTransferEvent::Complete:
            return false;
            /* handle me */
    }

    return false;
}

bool
cobraTransferController::addTransfer(cobraTransferFile* file)
{
    if (!file)
        return false;

    file->activate(false);

    m_vcftTransfers.push_back(file);
    return true;
}

int
cobraTransferController::recieveChunk(cobraTransferEvent* event)
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
    /* THis function may return
       ** TransferComplete
       ** TransferIncomplete (but contiguous)
       ** TransferIncompleteNoncontiguous
       */
    return false;
}

bool
cobraTransferController::addPendingTransfer(cobraTransferFile* file)
{
    if (m_vcftPending.contains(file->uid()))
        return false;

    m_vcftPending[file->uid()] = file;
    return true;
}

cobraTransferFile*
cobraTransferController::getPendingTransfer(uint32_t uid, const QByteArray& hash)
{
    cobraTransferFile* file = m_vcftPending[uid];
    if (file->hash() == hash) {
        m_vcftPending.remove(uid);
        return file;
    }

    return NULL;
}

QMap<uint32_t, cobraTransferFile*> cobraTransferController::m_vcftPending;
