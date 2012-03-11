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
cobraTransferStatistics::getStatistics(cobraStatMap& map) const
{
    QReadLocker lock(&m_rwlStatLock);
    map = m_mumMetrics;
}

cobraTransferFile::cobraTransferFile(QString& path)
    :QFile(path), m_uiUid(~0x0), m_bPendingCompletion(false)
{
    m_uiUid = cobraTransferFile::nextUid();
}

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
    if (m_bSending && m_bActive) {
        open(ReadOnly);
    } else if (!m_bSending && m_bActive) {
        open(ReadWrite);
    } else
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
    if (sending) {
        m_baHash = hash();
        m_baExpectedHash = m_baHash;
    }
 }

bool
cobraTransferFile::isPendingCompletion() const
{
    return m_bPendingCompletion;
}

void
cobraTransferFile::setPendingCompletion(bool pending)
{
    m_bPendingCompletion = pending;
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
     if(!isActive())
         return false;

     if (pos() == size()) {
         debug(ERROR(CRITICAL), "At End!\n");
         setPendingCompletion(true);
         return false;
     }

     qint64 offset = pos();
     QByteArray data = read(chunk);
     if (pos() != offset + data.size())
        seek(offset + data.size());

     debug(ERROR(CRITICAL), "Current file sent: %llu ? %llu\n", pos(), size());

     cobraTransferEvent* event = new cobraTransferEvent();

     event->setCommand(cobraTransferEvent::Chunk); // Set command to chunk
     event->setUid(m_uiUid);
     event->setSource(m_idSource);              // Set source
     event->setDestination(m_idDestination);    // Set destination
     event->setOffset(offset);                  // Set the infile offset
     event->setData(data);                      // Set data to be sent
     event->setHash(m_baHash);                  // Set hash

     return thread->sendEvent(event);           // Send the chunk event
}

int
cobraTransferFile::recieveChunk(cobraTransferEvent* event)
{
    if (!event || m_bSending)
        return cobraTransferFile::TransferError;

    if(event->uid() != m_uiUid || event->hash() != expectedHash())
        return cobraTransferFile::TransferError;

    qint64 offset = event->offset();

    seek(offset);
    debug(LOW, "Is OPEN %d \n", isOpen());
    debug(LOW, "SIZE OF DATA %d \n", event->data().size());
    debug(LOW, "Written: %d \n",write(event->data()));
    debug(LOW, "File Size: '%llu'\n",this->size());
    flush();

    if(expectedHash() == currentHash()) {
        return cobraTransferFile::TransferComplete;
    }
    else
        return cobraTransferFile::TransferIncomplete;
}

bool
cobraTransferFile::transferComplete()
{
    debug(MED, "Completed Transfer of file... \n");
    activate(false);
    return false;
}

bool
cobraTransferFile::resendChunk(qint64 chunk, qint64 offset)
{
    setPendingCompletion(false);
    // TODO: This needs to update the chunk list to allow for this chunk to 
    // be hte next one sent on the next sendChunk call..
    return false;
}

void
cobraTransferFile::setExpectedHash(QByteArray& hash)
{
    m_baExpectedHash = hash;
}

QByteArray
cobraTransferFile::expectedHash() const
{
    return m_baExpectedHash;
}

QByteArray
cobraTransferFile::hash()
{
    if (m_bSending) {
        if (m_baHash.isEmpty() || m_baHash.isNull())
            return currentHash();
        debug(ERROR(CRITICAL), "Hash: %s\n", m_baHash.toHex().data());
        return m_baHash;
    }

    return m_baExpectedHash;
}

QByteArray
cobraTransferFile::currentHash()
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    QFile reFile(this->fileName());

    reFile.open(QIODevice::ReadOnly);

    while(!reFile.atEnd()) {
        md5.addData(reFile.read(4096));
    }

    reFile.close();
    m_baHash = md5.result();

    return m_baHash;
}

int
cobraTransferFile::isComplete() const
{
    if (pos() >= size())
        return cobraTransferFile::TransferComplete;
\
    return cobraTransferFile::TransferIncomplete;
}

uint32_t
cobraTransferFile::nextUid()
{
    return cobraTransferFile::m_uiBaseUid++;
}

uint32_t cobraTransferFile::m_uiBaseUid = 0;

void
cobraTransferFile::setBaseUid(uint32_t base)
{
    cobraTransferFile::m_uiBaseUid = base;
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

cobraTransferFile*
cobraTransferController::getFile(uint32_t uid, const QByteArray& hash)
{
    for (int x=0; x<m_vcftTransfers.count(); x++) {
        if (!m_vcftTransfers[x]) {
            m_vcftTransfers.remove(x);
            x--;
            debug(ERROR(CRITICAL), "Null file found at %d\n", x);
            continue;
        }

        debug(LOW, "UID: %d - %d\n", uid, m_vcftTransfers[x]->uid());
        debug(LOW, "Hash: %s\n", hash.toHex().data());
        debug(LOW, "Hash: %s\n", m_vcftTransfers[x]->expectedHash().toHex().data());
        debug(LOW, "FILE: %d\n", x);
        if (!m_vcftTransfers[x]->is(uid))
            continue;

        debug(LOW, "Hash: %s\n", hash.toHex().data());
        debug(LOW, "Hash: %s\n", m_vcftTransfers[x]->expectedHash().toHex().data());
        if (m_vcftTransfers[x]->expectedHash() == hash)
            return m_vcftTransfers[x];
    }
    return NULL;
}

bool
cobraTransferController::transferTrigger()
{
    if (m_iNextTransfer < 0) {
        return true;
    }

    if (!m_vcftTransfers.size()) {
        m_iNextTransfer = -1;
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

    if (!file->isPendingCompletion()) {
        if (!file->isActive())
            file->activate(true);

        if (!file->sendChunk(m_netParent, m_iChunkSize))
            file->activate(false);
    } else {
        debug(ERROR(CRITICAL), "Pending Mother Fucking Completion!\n");
    }

    m_iNextTransfer++;
    return true;
}

bool
cobraTransferController::initialize(cobraNetEventThread* parent)
{
    m_netParent = parent;

    QObject::connect(&m_tTransferTimer, SIGNAL(timeout()), this, SLOT(transferTrigger()));

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

    cobraTransferFile* file = getFile(event->uid(), event->hash());
    if (!file)
        return false;

    switch (event->command()) {
        case cobraTransferEvent::Resend:
            file->resendChunk(event->size(), event->offset());
            return true;

        case cobraTransferEvent::Complete:
            file->transferComplete();
            return removeTransfer(file);
    }

    return false;
}

bool
cobraTransferController::addTransfer(cobraTransferFile* file)
{
    if (!file || file->is(~0x0UL))
        return false;

    debug(MED, "Adding file '%s' to transfer list.\n", 
	qPrintable(file->fileName()));
    file->activate(false);

    if (m_iNextTransfer < 0)
        m_iNextTransfer = 0;

    m_vcftTransfers.push_back(file);
    return true;
}

bool
cobraTransferController::removeTransfer(cobraTransferFile* file)
{
    debug(MED, "Removing file from transfer list.\n");

    for (int x=0; x<m_vcftTransfers.count(); x++) {
        if (!m_vcftTransfers[x]) {
            m_vcftTransfers.remove(x);
            x--;
            debug(ERROR(CRITICAL), "Null file found at %d\n", x);
            continue;
        }

        if (!m_vcftTransfers[x]->is(file->uid()))
            continue;

        if (m_vcftTransfers[x]->expectedHash() == file->currentHash()) {
            cobraTransferFile* delete_me = m_vcftTransfers[x];
            m_vcftTransfers.remove(x);
            delete delete_me;
            return true;
        }
    }

    return false;
}

int
cobraTransferController::recieveChunk(cobraTransferEvent* event)
{
    cobraTransferFile* file = getFile(event->uid(), event->hash());

    if (!file) {
        debug(ERROR(CRITICAL), "Failed to get associated file!\n");
        return cobraTransferFile::TransferError;
    }

    if (!file->isActive())
        file->activate(true);

    return  file->recieveChunk(event);
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

    if (file->expectedHash() == hash) {
        m_vcftPending[uid] = NULL;
        m_vcftPending.remove(uid);
        return file;
    }

    return NULL;
}

QMap<uint32_t, cobraTransferFile*> cobraTransferController::m_vcftPending;
