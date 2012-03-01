#include "debug.h"
#include "net.h"

cobraStateEvent::cobraStateEvent()
    :cobraNetEvent(cobraStateEventType), m_iFlags(0)
{}

cobraStateEvent::cobraStateEvent(cobraStateEvent& state)
    :cobraNetEvent(state),
      m_iFlags(state.m_iFlags), m_iState(state.m_iState)
{}

cobraStateEvent::~cobraStateEvent()
{}

int
cobraStateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    connection << m_iState;
    connection << m_iFlags;
    debug(CRITICAL, "Serialize State: %d\n", m_iState);
    return (size + sizeof(m_iState) + sizeof(m_iFlags));
}

int
cobraStateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    connection >> m_iState;
    connection >> m_iFlags;
    debug(CRITICAL, "Deserialize State: %d\n", m_iState);
    return (size + sizeof(m_iState) + sizeof(m_iFlags));
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
    cobraNetHandler* handler = cobraNetHandler::instance();

    switch(state->getState())
    {
    case cobraStateEvent::ConnectingState:
        {
            cobraAuthEvent* auth = new cobraAuthEvent();
            QString user = cobraNetHandler::instance()->getUsername();
            QString pwd = cobraNetHandler::instance()->getPassword();

            auth->setDestination(SERVER);
            auth->setResponse(false);
            auth->setUsername(user);
            auth->setPassword(pwd);

            debug(CRITICAL, "Sending Auth Event\n");
            handler->setConnectionState(cobraStateEvent::ConnectingState);
            handler->sendEvent(auth);
            break;
        }

    case cobraStateEvent::ConnectedState:
        {
            debug(MED, "Connection Accepted\n");
            handler->setId(state->destination());
            handler->setConnectionState(cobraStateEvent::ConnectedState);
            break;
        }

    case cobraStateEvent::ClosingState:
        {
            debug(MED, "Closing Connection\n");
            handler->reject();
            handler->setConnectionState(cobraStateEvent::ClosingState);
            break;
        }

    case cobraStateEvent::DisconnectedState:
        {
            debug(MED, "Disconnected from Server\n");

            bool wasConnected = handler->isConnected();
            handler->setConnectionState(cobraStateEvent::DisconnectedState);
            handler->reject();

            cobraChatEvent* chat = new cobraChatEvent();

            if (wasConnected)
                handler->chatNotify(SERVER, cobraMyId, CHAT_NOTIFY("Disconnected from server!"));
            else
                handler->chatNotify(SERVER,
                                    cobraMyId, CHAT_NOTIFY("Failed to connect to server!"));

            chat->setResponse(true);
            chat->setSource(SERVER);
            chat->setDestination(cobraMyId);
            handler->sendEvent(chat);

            chat = new cobraChatEvent();
            chat->setMsg("");
            chat->setCommand(cobraChatEvent::ListUpdate);
            chat->setResponse(true);
            chat->setSource(SERVER);
            chat->setDestination(cobraMyId);
            handler->sendEvent(chat);
            break;
        }

    case cobraStateEvent::ConnectionRefused:
        {
            debug(MED, "Invalid Credentials\n");
            handler->reject();
            handler->setConnectionState(cobraStateEvent::DisconnectedState);
            handler->removeConnection(SERVER);
            break;
        }
    }

    return true;
}

bool
cobraStateEventHandler::handleServerEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraStateEventType)
        return false;

    debug(CRITICAL, "Handling Client State Event: %lu\n", (unsigned long)QThread::currentThreadId());

    cobraStateEvent* state = dynamic_cast<cobraStateEvent*>(event);
    if (!state)
        return false;

    if (state->getState() == cobraStateEvent::DisconnectedState)
        cobraNetHandler::instance()->removeConnection(event->source());

    return true;
}

cobraNetEvent*
cobraStateEventHandler::eventGenesis() const
{
    return new cobraStateEvent();
}
