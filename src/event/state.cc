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
    return (size + sizeof(m_iState));
}

int
cobraStateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    connection >> m_iState;
    connection >> m_iFlags;
    debug(CRITICAL, "Deserialize State: %d\n", m_iState);
    return (size + sizeof(m_iState));
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

    switch(state->getState())
    {
    case cobraStateEvent::ConnectingState:
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

    case cobraStateEvent::ConnectedState:
        {
            cobraNetHandler::instance()->setId(state->destination());
            debug(MED, "Connection Accepted\n");
            cobraNetHandler::instance()->setConnected(true);
            break;
        }

    case cobraStateEvent::ClosingState:
        {
            debug(MED, "Closing Connection\n");
            cobraNetHandler::instance()->setConnected(false);
            break;
        }

    case cobraStateEvent::DisconnectedState:
        {
            debug(MED, "Disconnected from Server\n");
            cobraNetHandler::instance()->setConnected(false);

            cobraChatEvent* chat = new cobraChatEvent();
            chat->setMsg(CHAT_NOTIFY("Disconnected from server!"));
            chat->setResponse(true);
            chat->setSource(SERVER);
            chat->setDestination(cobraMyId);
            cobraSendEvent(chat);
            break;
        }

    case cobraStateEvent::ConnectionRefused:
        {
            debug(MED, "Invalid Credentials\n");
            cobraNetHandler::instance()->setConnected(false);
            cobraNetHandler::instance()->reject();
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
