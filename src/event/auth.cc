#include "debug.h"
#include "net.h"

#include <QApplication>

cobraAuthEvent::cobraAuthEvent()
    :cobraNetEvent(cobraAuthEventType)
{}

cobraAuthEvent::cobraAuthEvent(cobraAuthEvent& event)
    :cobraNetEvent(event), m_sUsername(event.m_sUsername),
      m_sPassword(event.m_sPassword)
{}

cobraAuthEvent::~cobraAuthEvent()
{}

int cobraAuthEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    debug(CRITICAL, "Serializing Auth Packet! %s:%s\n", qPrintable(m_sUsername), qPrintable(m_sPassword));
    connection << m_sUsername;
    connection << m_sPassword;
    return size + m_sUsername.length() + m_sPassword.length();
}

int cobraAuthEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    connection >> m_sUsername;
    connection >> m_sPassword;
    return size + m_sUsername.length() + m_sPassword.length();
}

QString cobraAuthEvent::username() const
{
    return m_sUsername;
}

void cobraAuthEvent::setUsername(const QString &user)
{
    m_sUsername = user;
}

QString cobraAuthEvent::password() const
{
    return m_sPassword;
}

void cobraAuthEvent::setPassword(const QString &passwd)
{
    m_sPassword = passwd;
}

cobraNetEvent*
cobraAuthEvent::duplicate()
{
    cobraAuthEvent* auth = new cobraAuthEvent(*this);
    return auth;
}

cobraAuthEventHandler::cobraAuthEventHandler()
    :cobraNetEventHandler("Auth", cobraAuthEventType)
{}

cobraAuthEventHandler::cobraAuthEventHandler(cobraAuthEventHandler& event)
    :cobraNetEventHandler(event)
{}

cobraAuthEventHandler::~cobraAuthEventHandler()
{}

bool
cobraAuthEventHandler::handleEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraAuthEventType)
        return false;

    if (event->isRequest())
        return handleServerEvent(event);

    debug(ERROR(CRITICAL), "Shouldn't got an Auth Response...\n");
    return true;
}

bool
cobraAuthEventHandler::handleServerEvent(cobraNetEvent* event)
{
    debug(CRITICAL, "Handling Client Authorization Request!\n");

    cobraAuthEvent* auth = static_cast<cobraAuthEvent*>(event);
    cobraNetHandler* netHandler = cobraNetHandler::instance();
    cobraStateEvent* newState = new cobraStateEvent();

    debug(CRITICAL, "Username: %s Password: %s\n", qPrintable(auth->username()), qPrintable(auth->password()));
    int authorized = netHandler->isAuthorized(auth->password());

    newState->setDestination(auth->source());
    newState->setSource(SERVER);

    if (authorized) {
        debug(HIGH, "User is authorized as a %s\n", authorized&GuestAuth?"Guest":"Participant");

        QString user = auth->username();

        debug(HIGH, "Username Authorized: %s\n", qPrintable(user));
        debug(HIGH, "ID: %d\n", event->source());

        netHandler->setIdUsername(event->source(), user);
        netHandler->setIdAuthorization(event->source(), authorized);
        netHandler->broadcastUserlist();

        newState->setState(cobraStateEvent::ConnectedState);
        cobraSendEvent(newState);
    } else {
        newState->setState(cobraStateEvent::ConnectionRefused);

        cobraSendEvent(newState);
        netHandler->removeConnection(auth->source());
    }

    return true;
}

cobraNetEvent*
cobraAuthEventHandler::eventGenesis() const
{
    return new cobraAuthEvent();
}

