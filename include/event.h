#ifndef EVENT_H
#define EVENT_H

#include <QEvent>
#include <QString>
#include <QtGui>
#include <QtNetwork/QAbstractSocket>

#define cobraNetEventType           (QEvent::User+1)
#define cobraStateEventType         (cobraNetEventType+1)
#define cobraAuthEventType          (cobraNetEventType+2)
#define cobraChatEventType          (cobraNetEventType+3)
#define cobraTransferEventType      (cobraNetEventType+4)
#define cobraNetEventTypeMax        (cobraNetEventType+5)

inline bool
validEvent(int type) {
    return (type > cobraNetEventType && type < cobraNetEventTypeMax);
}

/**
 * @typedef cobraId event.h "event.h"
 *
 * typedef for the id value used to track connections
 */
typedef unsigned int cobraId;

#define NO_ID           (~0x0U)
#define NO_SERVER       (~0x1U)
#define BROADCAST       (~0x2U)
#define SERVER          (~0xecc8ecc8U)
#define COBRA_ID_START  (0x10)

class cobraNetConnection;

#define CHAT_NOTIFY(x) "<font color=grey>" x "</font>"

 /**
  * @class cobraNetEvent event.h "event.h"
  *
  * The base class for handling events given particular parent cobraNetConnection.
  * If the data being recieved is a large amount of data, pull the necessary data,
  * and write the rest directly to disk (file transfers, primarily).
  */
class cobraNetEvent : public QEvent {
public:
    cobraNetEvent(int type);
    cobraNetEvent(cobraNetEvent& event);
    virtual ~cobraNetEvent();

    void setSource(cobraId id) { m_idSource = id; }
    cobraId source() const { return m_idSource; }
    void setDestination(cobraId id) { m_idDestination = id; }
    cobraId destination() const { return m_idDestination; }

    /**
     * @fn bool handled()
     * Indicate whether the event is being processed via QT Event system.  If so,
     * then the event will be freed automatically at the end of the current event cycle,
     * otherwise, it will be freed when its reference count reaches 0.
     * @param set Value to set the handled flag to.
     */
    bool handled() const;

    /**
     * @fn void setHandled(bool set)
     * Indicate whether the event is being processed via QT Event system.  If so,
     * then the event will be freed automatically at the end of the current event cycle.
     * Set to false to indicate that the event should be cleaned up when its reference count
     * reaches 0.
     * @param set Value to set the handled flag to.
     */
    void setHandled(bool set);

    /**
     * @fn int get(int cnt)
     * This is used to get an instance of an event before it has been sent
     * for handled. This really is only intended for calls which result in
     * the event getting sent to a thread through an InvokeMethod call.
     * @param cnt The number of elements to get.
     * @return The number of elements currently holding the object; -1 if handled.
     */
    int get(int cnt = 1);

    /**
     * @fn int put()
     * This is used to put an instance of an event after it has been handled.
     * When handled by the QTEvent system the Event will be cleaned up the
     * iteration after the current iteration in the main loop.  As a result,
     * this should really only be used on events which are not passed directly
     * into the QT Event system!
     * @return The number of elements currently holding the object; -1 if handled.
     */
    int put();

    /**
     * @fn bool isResponse() const
     * Used to determine if a given event is directed toward a client or server.
     * @return true if the event is a response.
     */
    bool isResponse() const {
        return m_bResponse;
    }

    /**
     * @fn void setResponse(bool is)
     * Set whether this even is intended
     * @return true if the event is a response.
     */
    void setResponse(bool isresponse) {
        m_bResponse = isresponse;
    }

    /**
     * @fn bool isRequest() const
     * Used to determine if a given event is directed toward a client or server.
     * @return true if the event is a request.
     */
    bool isRequest() const {
        return !m_bResponse;
    }

    /**
     * @fn virtual int serialize(QDataStream& stream)
     * Serialize the data which is pending on the given cobraNetConnection.
     */
    virtual int serialize(QDataStream& stream);

    /**
     * @fn virtual int deserialize(QDataStream& stream)
     * Deserialize the data which is pending on the given cobraNetConnection.
     */
    virtual int deserialize(QDataStream& stream);

    /**
     * @fn virtual cobraNetEvent* duplicate()
     * Create a new duplicate event and dump all values to it!
     * This is called to ensure that the main event loop or thread loop
     * doesn't delete it while we are using it, as once posted, the event loop
     * handles deletion.
     * Note: Sub-classes of cobraNetEvent only need to use the copy constructor
     * and then safely cleanup any internal data.
     * @return A pointer to the copied cobra event.
     */
    virtual cobraNetEvent* duplicate() = 0;

protected:
    /* Ref count is only used when m_bHandled is false! */
    QSemaphore  m_semRefcount;
    bool        m_bHandled;
    bool        m_bResponse;
    cobraId     m_idDestination;    /* Server ID */
    cobraId     m_idSource;         /* My ID */
};

/**
 * @class cobraNetEventHandler event.h "event.h"
 *
 * The cobraNetEventHandler is used to handle specific network events.
 * It may do this in the current thread or in its own thread, but
 * and interaction done with UI elements MUST be done in the MainThread
 * (QApplication::instance()->thread()).  If the handler sends a
 * received event to a handler thread, it should return that the event
 * was handle successfully, at which point, the recieving thread needs
 * to be notified (emit should work nicely) and the that thread must
 * then handle deletion of the memory.
 *
 * <b>NOTE: ONLY cobraNetEvent derivatives can be handled by cobraNetEventHandlers.
 *       If this rule is not obeyed, there may be issues with deletion of the events.</b>
 */
class cobraNetEventHandler {

public:
    cobraNetEventHandler(QString name, int event);
    cobraNetEventHandler(cobraNetEventHandler& event);
    virtual ~cobraNetEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event) = 0;

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const = 0;

    /**
     * @fn QString name()
     * Getter function for a human readable handler name.
     * @return QString name of the handler.
     */
    QString name() { return m_sName; }

    /**
     * @fn int type()
     * Getter function for the type of even this handler can handle.
     * @return QString name of the handler.
     */
    int     type() { return m_iType; }

    /**
     * @fn int put()
     * This may look funny because we're using the semaphore for reference counting.
     */
    int     put();

    /**
     * @fn int get()
     * This may look funny because we're using the semaphore for reference counting.
     */
    int     get();

protected:
    virtual bool handleServerEvent(cobraNetEvent* event) = 0;

protected:
    QString    m_sName;
    int        m_iType;
    QSemaphore m_semRef;
};

/**
 * @class cobraStateEvent event.h "event.h"
 *
 * The base class for handling events given a
 * particular parent cobraNetConnection.
 */

class cobraStateEvent : public cobraNetEvent {
public:
   cobraStateEvent();
   cobraStateEvent(cobraStateEvent& state);
   virtual ~cobraStateEvent();

public:

   /**
    * @fn void setState(int state)
    * Sets the state of the event.
    */
   void setState(int state) {
       m_iState = state;
   }

   /**
    * @fn int getState() const
    * Gets the current state.
    */
   int getState() const {
       return m_iState;
   }

   /**
    * @fn void setFlag(int flag)
    * Used to set a state flag.
    * @param flag The flag to be set.
    */
   void setFlag(int flag) {
       m_iFlags |= flag;
   }

   /**
    * @fn void clearFlag(int flag)
    * Used to clear a state flag.
    * @param flag The flag to be cleared.
    */
   void clearFlag(int flag) {
       m_iFlags &= ~flag;
   }

   /**
    * @fn void toggleFlag(int flag)
    * Used to toggle a state flag.
    * @param flag The flag to be toggle.
    */
   void toggleFlag(int flag) {
       m_iFlags ^= flag;
   }

   /**
    * @fn virtual int serialize(QDataStream& stream)
    * Serialize the data which is pending on the given cobraNetConnection.
    */
   virtual int serialize(QDataStream& stream);

   /**
    * @fn virtual int deserialize(QDataStream& stream)
    * Deserialize the data which is pending on the given cobraNetConnection.
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

   enum SocketState {
       ConnectingState,
       ConnectedState,
       ClosingState,
       ConnectionRefused,
       DisconnectedState
   };

   enum StateFlags {
       Forced = 0x1,
       AuthenticationFailure = 0x2
   };

protected:
   int  m_iFlags;
   int  m_iState;
};


/**
 * @class cobraStateEventHandler event.h "event.h"
 *
 * The cobraStateEventHandler is used to handle incoming state events.
 * Because this event handler interfaces with the User Interface, it
 * absolutly CAN NOT use threads to process incoming information.
 */
class cobraStateEventHandler : public cobraNetEventHandler {

public:
    cobraStateEventHandler();
    cobraStateEventHandler(cobraStateEventHandler& event);
    virtual ~cobraStateEventHandler();

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


/**
 * @class cobraChatEvent event.h "event.h"
 *
 * The event used to communicate chat information to the server and other
 * machines.
 */

class cobraChatEvent : public cobraNetEvent {
public:
   cobraChatEvent();
   cobraChatEvent(cobraChatEvent& state);
   virtual ~cobraChatEvent();

   QString msg() const;
   void setMsg(const QString& msg);

   int command() const;
   void setCommand(int cmd);

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

   enum ChatCommands {
       ChatMessage,
       ChangeUsername,
       Away,
       ListUpdate
   };

protected:
    int m_iCommand;
    QString m_sMessage;
};

/**
 * @class cobraAuthEventHandler event.h "event.h"
 *
 * The cobraAuthEventHandler is used to handle incoming authorization requests.
 */
class cobraAuthEventHandler : public cobraNetEventHandler {

public:
    cobraAuthEventHandler();
    cobraAuthEventHandler(cobraAuthEventHandler& event);
    virtual ~cobraAuthEventHandler();

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

/**
 * @class cobraAuthEvent event.h "event.h"
 *
 * The event used to communicate authentication information to the server and other
 * machines.
 */

class cobraAuthEvent : public cobraNetEvent {
public:
   cobraAuthEvent();
   cobraAuthEvent(cobraAuthEvent& state);
   virtual ~cobraAuthEvent();

   QString username() const;
   void setUsername(const QString& user);

   QString password() const;
   void setPassword(const QString& pwd);

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
protected:
   QString m_sUsername;
   QString m_sPassword;
};

/**
 * @class cobraChatEventHandler event.h "event.h"
 *
 * The cobraChatEventHandler is used to handle incoming chat events.
 * Because this event handler interfaces with the User Interface, it
 * absolutly CAN NOT use threads to process incoming information.
 */
class cobraChatEventHandler : public cobraNetEventHandler {

public:
    cobraChatEventHandler();
    cobraChatEventHandler(cobraChatEventHandler& event);
    virtual ~cobraChatEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

    bool            setChatDock(QDockWidget* chat);

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);

protected:
    QDockWidget*    m_dwChatDock;
    QPushButton*    m_pbSend;
    QTextEdit*      m_teText;
    QTextEdit*      m_teChat;
    QListWidget*    m_lwUserlist;
};



/**
 * @class cobraTransferEventHandler event.h "event.h"
 *
 * The cobraTransferEventHandler is used to handle incoming authorization requests.
 */
class cobraTransferEventHandler : public cobraNetEventHandler {

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
protected:
};



#endif // EVENT_H
