#ifndef TRANSFER_H
#define TRANSFER_H

#include "event.h"

class cobraTransferFile;

/**
 * @class cobraTransferEvent event.h "event.h"
 *
 * The event used to communicate authentication information to the server and other
 * machines.
 */

class cobraTransferEvent : public cobraNetEvent {
public:
   cobraTransferEvent();
   cobraTransferEvent(cobraTransferEvent& state);
   virtual ~cobraTransferEvent();

   void setCommand(int cmd);
   int command() const;

   void setUid(uint32_t uid);
   uint32_t uid() const;

   void setOffset(qint64 uid);
   qint64 offset() const;

   void setSize(qint64 uid);
   qint64 size() const;

   /* This should contain the HASH of the file to be sent. */
   void setHash(QByteArray& hash);
   const QByteArray& hash() const;

   void setData(QByteArray& data);
   const QByteArray& data() const;

   bool fromFile(cobraTransferFile* file);

public:

   /**
    * @fn virtual int serialize(QDataStream& stream)
    * Serialize the data for chats which is pending on the given cobraNetConnection.
    */
   virtual int serialize(QDataStream& stream);

   /**
    * @fn virtual int deserialize(QDataStream& stream)
    * Deserialize the data for chats which is pending on the given cobraNetConnection.
    */
   virtual int deserialize(QDataStream& stream);

   /**
    * @fn virtual cobraNetEvent* duplicate()
    * Create a new duplicate event and dump all values to it!
    * This is called to ensure that the main event loop or thread loop
    * doesn't delete it while we are using it, as once posted, the event loop
    * handles deletion.
    * @return A pointer to the copied cobra event.
    */
   virtual cobraNetEvent* duplicate();

   enum TransferCommands {
       Request,
       Accept,
       Reject,
       Chunk,
       Complete,
       Resend
   };

protected:
    int             m_iCommand;
    uint32_t        m_uiUid;
    qint64          m_iOffset;
    qint64          m_iSize;
    QByteArray      m_baHash;
    QByteArray      m_baData;
};

class cobraNetEventThread;
class cobraTransferController;

#define CURRENT_OFFSET (-1)

/**
 * @class cobraTransferFile event.h "event.h"
 *
 * The cobraTransferFile class is intended to provide necessary functions to make generating
 * file transfer events easier.  The goal is to use this object to store the path to the file
 * prior to activating the transfer event.
 * Outbound:
 * Once a file transfer is started, this class will handle reading from the file and sending
 * the data via an event to its destination.
 * Inbound:
 * Once a file transfer is started, this class will handle writing incoming events to the file.
 *
 * Each file transfer will have its own unique ID generated by the sending element.
 */
class cobraTransferFile : public QFile {
    Q_OBJECT

public:

    cobraTransferFile(QString&);
    ~cobraTransferFile();

    uint32_t uid() const;
    void setUid(uint32_t uid);
    bool isTransfer(uint32_t uid) const;

    bool isActive() const;
    void activate(bool enable = true);

    bool isSending() const;
    void setSending(bool sending = true);

    bool is(uint32_t id) const;

    void setExpectedHash(QByteArray& hash);
    QByteArray expectedHash() const;
    QByteArray hash();
    QByteArray currentHash();

    void setDestination(cobraId dest);
    void setSource(cobraId src);
    cobraId destination() const;
    cobraId source() const;

    bool sendChunk(cobraNetEventThread* thread, qint64 chunk = 1024);
    int recieveChunk(cobraTransferEvent* event);

    enum ChunkReceipt {
        TransferError = 0,
        TransferComplete = 1,
        TransferIncomplete = 2,
        TransferIncompleteNoncontiguous = 3
    };

    int isComplete() const;

    bool transferComplete();
    bool resendChunk(qint64 chunk, qint64 offset);

protected:
    uint32_t    m_uiUid;

    QByteArray  m_baHash;
    QByteArray  m_baExpectedHash;

    cobraId     m_idSource;
    cobraId     m_idDestination;

    bool        m_bActive;
    bool        m_bSending;
};

/**
 * @class cobraTransferController event.h "event.h"
 *
 * The class is used by the file transfer API to keep track of file transfer operations.
 * Internal this object is all pending outbound file transfers.  File transfers are
 * allowed to schedule in queue, which only allows a certain number of files open in
 * the queue at a time.  Once a file transfer has completed, the next file in the
 * pending list is opened, and the transfer is started.  The inbound tansfer mechanism is
 * handled as well, but is controlled via the cobraTransferFileController object in the
 * cobraTransferEventHandler class.
 *
 */
class cobraTransferController : public QObject {
    Q_OBJECT

public:
   cobraTransferController(int concurrent = cobraConcurrentTransfers, QObject* parent = NULL);
   virtual ~cobraTransferController();

public slots:
   bool processTrigger();

public:

   void setTransferCount(int con);
   int transferCount() const;

   void setChunkSize(qint64 chunk);
   qint64 chunkSize() const;

   void setInterval(int interval);
   int interval() const;

   int recieveChunk(cobraTransferEvent* event);

   bool interceptEvent(cobraTransferEvent* event);
   bool addTransfer(cobraTransferFile* file);

   bool initialize(cobraNetEventThread* parent = NULL);
   void cleanup();

protected:
   cobraTransferFile* getFile(uint32_t uid, const QByteArray& hash) const;

protected:
   int                          m_iNextTransfer;
   int                          m_iConcurrentTransfers;
   qint64                       m_iChunkSize;

   cobraNetEventThread*         m_netParent;
   QTimer                       m_tTransferTimer;
   QVector<cobraTransferFile*>  m_vcftTransfers;

public:
   static bool addPendingTransfer(cobraTransferFile* file);
   static cobraTransferFile* getPendingTransfer(uint32_t uid, const QByteArray& hash);
   static QMap<uint32_t, cobraTransferFile*>  m_vcftPending;
};


/**
 * @class cobraTransferEventHandler event.h "event.h"
 *
 * The cobraTransferEventHandler is used to handle incoming authorization requests.
 */
class cobraTransferEventHandler : public cobraNetEventHandler, public cobraTransferController {

public:
    cobraTransferEventHandler();
    cobraTransferEventHandler(cobraTransferEventHandler& event);
    virtual ~cobraTransferEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);
};

class cobraTransferStatistics {
public:
    cobraTransferStatistics();
    ~cobraTransferStatistics();

    struct metrics {
        QString path;
        unsigned long size;
        unsigned long current;
        unsigned long time;
    };

    typedef QMap<uint32_t, metrics> cobraStatMap;

    static cobraTransferStatistics* instance();

    void updatePath(uint32_t uid, QString path);
    void updateSize(uint32_t uid, unsigned long size);
    void updateCurrent(uint32_t uid, unsigned long current);
    void updateTime(uint32_t uid, unsigned long time);
    void getStatistics(cobraStatMap& map) const;

protected:
    mutable QReadWriteLock  m_rwlStatLock;
    cobraStatMap            m_mumMetrics;
};


#endif // TRANSFER_H
