#include "debug.h"
#include "net.h"

#include <QApplication>

cobraChatEvent::cobraChatEvent()
    :cobraNetEvent(cobraChatEventType), m_iCommand(Message)
{}

cobraChatEvent::cobraChatEvent(cobraChatEvent& event)
    :cobraNetEvent(event), m_iCommand(event.m_iCommand), m_sMessage(event.m_sMessage)
{}

cobraChatEvent::~cobraChatEvent()
{}

int cobraChatEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    connection << m_iCommand;
    connection << m_sMessage;
    return size + m_sMessage.length() + sizeof(m_iCommand);
}

int cobraChatEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    connection >> m_iCommand;
    connection >> m_sMessage;
    return size + m_sMessage.length() + sizeof(m_iCommand);
}

QString
cobraChatEvent::msg() const
{
    return m_sMessage;
}

void
cobraChatEvent::setMsg(const QString &msg)
{
    m_sMessage = msg;
}

int
cobraChatEvent::command() const
{
    return m_iCommand;
}

void
cobraChatEvent::setCommand(int cmd)
{
    m_iCommand = cmd;
}

cobraNetEvent*
cobraChatEvent::duplicate()
{
    cobraChatEvent* chat = new cobraChatEvent(*this);
    return chat;
}

cobraChatEventHandler::cobraChatEventHandler()
    :cobraNetEventHandler("Chat", cobraChatEventType),
      m_dwChatDock(NULL), m_pbSend(NULL),
      m_teText(NULL), m_teChat(NULL)

{}

cobraChatEventHandler::cobraChatEventHandler(cobraChatEventHandler& event)
    :cobraNetEventHandler(event),
      m_dwChatDock(event.m_dwChatDock), m_pbSend(event.m_pbSend),
      m_teText(event.m_teText), m_teChat(event.m_teChat)
{}

cobraChatEventHandler::~cobraChatEventHandler()
{}

bool
cobraChatEventHandler::handleEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraChatEventType)
        return false;

    cobraChatEvent* ev = static_cast<cobraChatEvent*>(event);
    if (ev->isRequest())
        return handleServerEvent(event);

    debug(CRITICAL, "Handling client chat request!\n");

    if (m_teChat) {
        m_teChat->append(ev->msg());
    }

    return true;
}

bool
cobraChatEventHandler::handleServerEvent(cobraNetEvent* event)
{
    debug(CRITICAL, "Handling client chat request!\n");

    cobraChatEvent* chat = static_cast<cobraChatEvent*>(event);

    chat->setResponse(true);
    chat->setDestination(BROADCAST);
    chat->setSource(SERVER);

    cobraSendEvent(chat);
    return true;
}

cobraNetEvent*
cobraChatEventHandler::eventGenesis() const
{
    debug(CRITICAL, "Genesis\n");
    return new cobraChatEvent();
}

bool
cobraChatEventHandler::setChatDock(QDockWidget* chat)
{
    if (m_dwChatDock)
        return false;

    m_dwChatDock = chat;

    m_pbSend = chat->findChild<QPushButton *>("sendButton");
    m_teText = chat->findChild<QTextEdit *>("sendText");
    m_teChat = chat->findChild<QTextEdit *>("chatText");

    return true;
}
