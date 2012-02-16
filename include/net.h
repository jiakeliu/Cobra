#ifndef NET_H
#define NET_H

#include <QApplication>
#include <QThread>

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>

#include "event.h"
#include "uperms.h"
/**
 * Event Driven Network Handler
 *
 * In order to more effectively use Qt's provided classes,
 * the network handler will use the QMainWindow that generates it
 * as the base object, and all cobraEvents will be derived from the
 * parent cobraEvent class.
 *
 * Network Interactions:
 *      Get cobreNetHandler Singleton.
 *      Allocate and configure cobreNetConnection.
 *      Add to cobraNetHandler Singleton.
 *      Call cobreaNetHandler Init.
 *
 *      Client:
 *          On connect, cobraStateEvent(Connected, serverId);
 *          On recieve, cobra****Event(data);
 *          On send, QApplication::sendEvent(cobraNetHandler::instance(), cobra****Event());
 *          On disconnect, sendEvent(cobraStateEvent(Disconnected,SessionID));
 *
 *      Server:
 *          On connect, cobraNetHandlerThreadDispatcher->dispatchSocket(fd);
 *                      sendEvent(cobraStateEvent(Connected,cobraId,serverId));
 *          On recieve, sendEvent(cobra****Event(data));
 *          On send, cobraNetHandler->sendRequest(cobra****Event());
 *          On disconnect
 *
 */

#define IPV4_REGEX "\b((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.|$)){1,4}\b"
#define IPV6_REGEX "\b(([A-Fa-f0-9]{1-4})(:|::$)){0,8}\b"
#define HOST_NAME  "\b(([a-zA-Z0-9-]+)(\.|$))+\b"

#define cobraNetEventCnxPerThread   (2)
#define cobraNetEventThreadCount    (4)

#define cobraStreamMagic            (0x31337313)

class cobraNetConnection;
extern QSettings* g_cobra_settings;

 /**
  * @class cobraNetEventThread net.h "net.h"
  *
  * This class handles connections on a set of sockets.
  */
class cobraNetEventThread : public QObject {
    Q_OBJECT

public:
    cobraNetEventThread(QObject* parent = NULL, int cnx = cobraNetEventCnxPerThread);
    virtual ~cobraNetEventThread();

public slots:
    bool connectionRequest(int fd, int id);
    bool connect(QString ip, int port);

    void disconnect();
    int readyRead();
    int sockError(QAbstractSocket::SocketError);
    int sslErrors(QList<QSslError>);
    void serverReady();
    void clientReady();

    bool sendEvent(cobraNetEvent* event);

 public:

    /**
     * @fn int connections() const
     * @return The number of connections currently on a given thread.
     */
    int connections() const {
        return (m_iMaxConnections - m_semAvailableConnections.available());
    }

 protected:
    QVector<cobraNetConnection*>    m_cncConnections;
    QSemaphore                      m_semAvailableConnections;
    int                             m_iMaxConnections;
};


/**
 * @class cobraNetConnection net.h "net.h"
 */
class cobraNetConnection : public QSslSocket {
    Q_OBJECT

public:
    cobraNetConnection(QObject *parent = NULL);
    virtual ~cobraNetConnection();

   void setNewId();
   void setId(cobraId id);
   static cobraId getNewId();

public:
    cobraId id() const;

    bool is(cobraId id);

protected:
    cobraId     m_Id;
    static cobraId currentId;
};


/**
 * @class cobraNetHandler net.h "nethandler."
 *
 * The cobraNetHandler handles all underlying network communication
 * but the server and client.  The individual components can be
 * initialized manualy or read in from a configuration file, via
 * setConfigFile.
 */
class cobraNetHandler : public QTcpServer {
    Q_OBJECT

public:
    cobraNetHandler(QObject* parent = NULL, int cnt = cobraNetEventThreadCount);
    virtual ~cobraNetHandler();

    /**
     * @fn static cobraNetHandler* cobraNetHandler::instance()
     * The cobraNetHandler::instance() function acts as a singleton object by
     * returning a pointer to a static cobraNetHandler object.
     * @return A pointer to the cobraNetHandler singleton.
     */
    static cobraNetHandler* instance();

    /**
     * @fn int connect(QString ip, int port)
     * Attempt to connect to the specified server.
     * @param ip Ip/Hostname of the server to connect to.
     * @param port Port to connect to on the server.
     * @param user Username to use to connect to the server.
     * @param password Password to use when connecting to the server.
     * @return true is connection inits correctly.
     */
    bool connect(QString ip, int port, QString user, QString password);

    /**
     * @fn bool broadcastSeverEvent(cobraNetEvent* event)
     * Sends the specified event to all systems connected.
     * @param event cobraNetEvent* to send.
     * @return true on successful send; false if send failed immediately.
     */
    bool broadcastServerEvent(cobraNetEvent* event);

    /**
     * @fn bool sendSeverEvent(cobraNetEvent* event)
     * Sends the specified event to the requested destination.
     * @param event cobraNetEvent* to send.
     * @return true on successful send; false if send failed immediately.
     */
    bool sendServerEvent(cobraNetEvent* event);

    /**
     * @fn bool triggerErrorEvent(cobraNetEvent* event)
     * Response with the current event notifying the sender of the error.
     * @param event The event that triggered the error.
     * @return True if the event error was succesfully handled.
     */
    //bool triggerErrorResponse(cobraNetEvent* event);

public:

    /**
     * @fn virtual bool event(QEvent * event)
     * Used to intercept cobraStateEvents to track connection state,
     * and to send out events that need to be send over the network.
     */
    virtual bool event(QEvent * event);

    /**
     * @fn virtual bool eventFilter(QObject *object, QEvent * event)
     * Used to intercept cobraNetEvents which might be sent to other
     * parts of the application...
     */
    virtual bool eventFilter(QObject *object, QEvent * event);

    /**
     * @fn bool registerEventHandler(cobraNetEventHandler* handler)
     * Used to register even thandlers.  Dupicates will be rejected.
     * Once registered, the cobraNetEventHandler will actively delete
     * all associated handlers on removal or destruction.
     * @param handler A pointer to the derived cobraNetEventHandler class.
     * @return false if the handler is a duplicate.
     */

    bool registerEventHandler(cobraNetEventHandler* handler);

    /**
     * @fn cobraNetEventHandler* getEventHandler(int type)
     * Get the event handler for the given type.
     * @param type The type of event handler to fetch.
     * @return NULL if doesn't exist; the handler if it does.
     */

    cobraNetEventHandler* getEventHandler(int type);

    /**
     * @fn cobraNetEvent* getEvent(int type)
     * Get the event for the given type.
     * @param type The type of event to fetch.
     * @return NULL if doesn't exist; the handler if it does.
     */

    cobraNetEvent* getEvent(int type);

    /**
     * @fn bool removeHandler(int type)
     * Remove the event handler associated with the specified type.
     * @param type The type of even handler to delete.
     * @return NULL if doesn't exist; the handler if it does.
     */
    bool removeEventHandler(int type);

    /**
     * @fn cobraId myId() const
     * @return The this applications cobraId, if connection.  NO_ID if not connected.
     */
    cobraId myId() const {
        return m_idMine;
    }

    /**
     * @fn bool isServing() const
     * @return Whether or not this net handler is serving or not.
     */
    bool isServing() const {
        return m_bServing;
    }

    void setUsername(QString user) {
        m_sUser = user;
    }

    void setPassword(QString password) {
        m_sPass = password;
    }

    QString getUsername() const {
        return m_sUser;
    }
    QString getPassword() const {
        return m_sPass;
    }

    /**
     * @fn int optimalThread() const
     * Find and return the index of the optimal thread to use, if one exists.
     * @return -1 if no thread found, index otherwise.
     */
    int optimalThread() const;

    virtual bool listen(const QHostAddress& address = QHostAddress::Any, qint16 port = 0);

    /**
     * @fn QSslCertificate getCaCertificate() const;
     * Return the CA Certificate to be used with a cobraNetConnection.
     * @return The global Ca Certificate.
     */
    QSslCertificate getCaCertificate() const;

    /**
     * @fn QSslCertificate getLocalCertificate() const;
     * Return the Local Certificate to be used with a cobraNetConnection.
     * @return The global Local Certificate.
     */
    QSslCertificate getLocalCertificate() const;

    /**
     * @fn QSslKet getPrivateKey() const;
     * Return the SSL Private Key to be used with a cobraNetConnection.
     * @return The global Ssl Private Key.
     */
    QSslKey getPrivateKey() const;

    /**
     * @fn bool loadClientCertificates();
     * Load the configured client certificates.
     * @return True if all certificates were found and successfully loaded.
     */
    bool loadClientCertificates();

    /**
     * @fn bool loadServerCertificates(QString password);
     * Load the configured server certificates.
     * @param password The password for the private key if one is specified.
     * @return True if all certificates/keys were found and successfully loaded.
     */
    bool loadServerCertificates(QString password);

    /**
     * @fn static bool loadCertificate(QString path, QSslCertificate& cert);
     * Load the specified certificate.
     * @param path The path to the pem encoded certificate.
     * @return True if the specified certificate was found and successfully loaded.
     */
    static bool loadCertificate(QString path, QSslCertificate& cert);

    /**
     * @fn static bool loadKey(QString path, QString password, QSslKey& key);
     * Load the specified certificate.
     * @param path The path to the pem encoded key.
     * @param password The password of the pem encoded key.
     * @return True if the specified key was found and successfully loaded.
     */
    static bool loadKey(QString path, QString password, QSslKey& key);

    /**
     * @fn int setAllowedErrors(QList<QSslError> err)
     * Set the errors which are to be allowed.
     * @param err Error list to be allowed.
     * @return Number of errors in the list.
     */
    int setAllowedErrors(QList<QSslError> err);

    /**
     * @fn int getAllowedErrors(QList<QSslError>& err)
     * Set the errors which are to be allowed.
     * @param err Error list to be transfered.
     * @return Number of errors in the list.
     */
    void getAllowedErrors(QList<QSslError>& err);

    /**
     * @fn bool setIdThread(cobraId id, int index)
     * Create an association between the specified id and index.
     * @param id The id to associate.
     * @param index The index to associate with the Id.
     */
    bool setIdThread(cobraId id, int index);


    /**
     * @fn void setId(cobraId id)
     * Set the handler ID to the specified id (Should be done by State Connected event).
     * @param id The id to associate.
     */
    void setId(cobraId id) {
        m_idMine = id;
    }

    /**
     * @fn bool isAuthorized(QString user, QString pass)
     * Validates the provided credentials against the configured list.
     * @param user Username sent by the client.
     * @param pass Password sent by the client.
     * @return True if a valid user account; False otherwise.
     */
    bool isAuthorized(QString user, QString pass);

    /**
     * @fn bool setSessionPassword(QString pwd)
     * Sets the server session password.
     * @param pwd The password value to assign to the session.
     * @return true if properly set; false otherwise
     */
    bool setSessionPassword(QString pwd);

    /**
     * @fn QString getSessionPassword() const
     * Gets the server session password.
     * @return The current session password string
     */
    QString getSessionPassword() const;

    /**
     * @fn void getAuthList(QList<UserPermissions> &authList) const
     * Returns the current list of Authorized persons.
     * @param authList Used to transfer the list of user permissions.
     */
    void getAuthList(QList<UserPermissions> &authList) const;

    /**
     * @fn void setAuthList(QList<UserPermissions> &authList)
     * Sets the current list of Authorized persons.
     * @param authList Used to transfer the list of user permissions.
     */
    void setAuthList(QList<UserPermissions> &authList);

    /**
     * @fn bool addAuth(UserPermissions &auth)
     * Adds the given UserPermissions object to the list of authorized users.
     * @param auth Used to transfer the user permissions object.
     * @return Returns true if added; false otherwise or duplicate user.
     */
    bool addAuth(UserPermissions &auth);

    /**
     * @fn bool delAuth(UserPermissions &auth)
     * Removes the given UserPermissions object from the list of authorized users.
     * @param auth Used to transfer the user permissions object.
     * @return Returns true if added; false otherwise or duplicate user.
     */
    bool delAuth(UserPermissions &auth);

public slots:

    /**
     * @fn int init()
     * Initialize all threads.
     * @return The number of threads created.
     */
    int init();

    /**
     * @fn void cleanup()
     * Cleanup all threads.
     */
    void cleanup();

protected:

    /**
     * @fn int incomingConnection(int desc)
     * incomingConnection is triggered whenever the server is recieves a connect request.
     * @param desc Th file descriptor that is associated with the socket.
     */
    void incomingConnection(int descriptor);

protected:
    typedef QMap<int, cobraNetEventHandler*>::Iterator         eventIterator;
    typedef QMap<int, cobraNetEventHandler*>::ConstIterator    constEventIterator;

    QMap<int, cobraNetEventHandler*>    m_mihHandlers;

    typedef QVector<cobraNetEventThread*>::Iterator         workerIterator;
    typedef QVector<cobraNetEventThread*>::ConstIterator    constWorkerIterator;

    QVector<cobraNetEventThread*>       m_cnetWorkers;
    QVector<QThread*>                   m_tThreads;

    QList<QSslError>                    m_lAllowedErrors;

    QString                             m_sUser;
    QString                             m_sPass;

    mutable QReadWriteLock              m_eventLock;
    mutable QReadWriteLock              m_certLock;
    mutable QReadWriteLock              m_errorLock;
    mutable QReadWriteLock              m_cidLock;

    QMap<cobraId, int>                  m_cIds;
    cobraId                             m_idMine;
    int                                 m_iThreadCnt;

    QSslCertificate                     m_sslCaCertificate;
    QSslCertificate                     m_sslLocalCertificate;
    QSslKey                             m_sslPrivateKey;

    QList<UserPermissions>             m_lupAuthorized;
    bool                                m_bServing;
};

/**
 * @fn inline int sendEvent(cobraNetEvent* event)
 *
 * Helper function used to send an event.
 */
inline void cobraSendEvent(cobraNetEvent* event) {
    QApplication::postEvent(cobraNetHandler::instance(), event);
}

#define cobraMyId (cobraNetHandler::instance()->myId())

#endif // NETHANDLER_H
