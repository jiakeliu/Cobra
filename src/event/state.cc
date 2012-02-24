#include "debug.h"
#include "net.h"

cobraStateEvent::cobraStateEvent()
    :cobraNetEvent(cobraStateEventType)
{}

cobraStateEvent::cobraStateEvent(cobraStateEvent& state)
    :cobraNetEvent(state), m_sState(state.m_sState)
{}

cobraStateEvent::~cobraStateEvent()
{}

int
cobraStateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    connection << m_sState;
    debug(CRITICAL, "Serialize State: %d\n", m_sState);
    return (size + sizeof(m_sState));
}

int
cobraStateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    connection >> m_sState;
    debug(CRITICAL, "Deserialize State: %d\n", m_sState);
    return (size + sizeof(m_sState));
}

cobraNetEvent*
cobraStateEvent::duplicate()
{
    cobraStateEvent* auth = new cobraStateEvent(*this);
    return auth;
}

cobraStateEventHandler::cobraStateEventHandler()
    :cobraNetEventHandler("State", cobraStateEventType)
{}

cobraStateEventHandler::cobraStateEventHandler(cobraStateEventHandler& event)
    :cobraNetEventHandler(event)
{}

cobraStateEventHandler::~cobraStateEventHandler()
{}

bool
cobraStateEventHandler::handleEvent(cobraNetEvent* event)
{
    if (event->isRequest())
        return handleServerEvent(event);

    cobraStateEvent* state = static_cast<cobraStateEvent*>(event);
    debug(CRITICAL, "Recieved Server State Event: %lu\n", (unsigned long)QThread::currentThreadId());

   
// we need to make our own enumeration
    switch(state->getState())
    {
    case ConnectingState:
        {
            cobraAuthEvent* auth = new cobraAuthEvent();
            QString user = cobraNetHandler::instance()->getUsername();
            QString pwd = cobraNetHandler::instance()->getPassword();

            auth->setDestination(SERVER);
            auth->setUsername(user);
            auth->setPassword(pwd);

            debug(CRITICAL, "Sending Auth Packet\n");
            cobraNetHandler::instance()->setConnected(true);
            cobraSendEvent(auth);
            break;
        }

    case ConnectedState:
    {
        cobraNetHandler::instance()->setId(state->destination());
        debug(MED, "Connection Accepted\n");
        cobraNetHandler::instance()->setConnected(true);

        QString user = cobraNetHandler::instance()->getUsername();
        QStringList list = cobraNetHandler::instance()->getUserList();

        //Temp user type
        QString type = "Host";

        if (type == "Host") {
            user.append("!");
            list << user;
        }
        else {
            user.append("*");
            list << user;
        }

        cobraNetHandler::instance()->updateUserList(list);

        break;
    }

    case ClosingState:
        {
            debug(MED, "Closing Connection\n");
            cobraNetHandler::instance()->setConnected(false);

            QString user = cobraNetHandler::instance()->getUsername();
            QStringList list = cobraNetHandler::instance()->getUserList();

            QString userHost = user.append("!");
            QString userGuest = user.append("*");

            list.removeAll(userHost);
            list.removeAll(userGuest);

            cobraNetHandler::instance()->updateUserList(list);

            break;
        }

    case DisconnectedState:
        {
            debug(MED, "Disconnected from Server\n");
//need to break all connections
            cobraNetHandler::instance()->setConnected(false);
            break;
        }

    case ConnectionRefused:
        {
            debug(MED, "Invalid Credentials\n");
//need to break all connections
            cobraNetHandler::instance()->setConnected(false);
            break;
        }
    }
    return false;
}

bool
cobraStateEventHandler::handleServerEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraStateEventType)
        return false;

    debug(CRITICAL, "Handling Client State Event: %lu\n", (unsigned long)QThread::currentThreadId());
    return true;
}

cobraNetEvent*
cobraStateEventHandler::eventGenesis() const
{
    return new cobraStateEvent();
}
