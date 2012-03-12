#include "debug.h"
#include "net.h"

#include <QApplication>

cobraClipUpdateEvent::cobraClipUpdateEvent()
    :cobraNetEvent(cobraClipUpdateEventType)
{}

cobraClipUpdateEvent::cobraClipUpdateEvent(cobraClipUpdateEvent& event)
    :cobraNetEvent(event), m_ccClip(event.m_ccClip)
{}

cobraClipUpdateEvent::~cobraClipUpdateEvent()
{}

int cobraClipUpdateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    debug(CRITICAL, "Serializing ClipUpdate Packet! %s:%s\n", qPrintable(QString::number(m_ccClip.getUid())), qPrintable(m_ccClip.getPath()));

    connection << m_iCommand;
    connection << QString::number(m_ccClip.getUid());
    connection << m_ccClip.getPath();
    connection << m_ccClip.getHash();
    connection << QString::number(m_ccClip.getSize());
    connection << m_ccClip.getModifiedTime();
    connection << m_ccClip.getTitle();
    connection << m_ccClip.getTags();
    connection << m_ccClip.getDescription();

    return size + QString::number(m_ccClip.getUid()).length() +
                  m_ccClip.getPath().length() +
                  m_ccClip.getHash().length() +
                  QString::number(m_ccClip.getSize()).length() +
                  m_ccClip.getModifiedTime().length() +
                  m_ccClip.getTitle().length() +
                  m_ccClip.getTags().length() +
                  m_ccClip.getDescription().length();
}

int
cobraClipUpdateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    QString tmpString;

    connection >> m_iCommand;
    size += sizeof(m_iCommand);

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setUid(tmpString.toInt());

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setPath(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setHash(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setSize(tmpString.toInt());

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setModifiedTime(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setTitle(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setTags(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setDescription(tmpString);

    return size;
}

cobraClip
cobraClipUpdateEvent::getClip() const
{
    return m_ccClip;
}

void
cobraClipUpdateEvent::setClip(const cobraClip &newClip)
{
    m_ccClip = newClip;
}

int
cobraClipUpdateEvent::getCommand() const
{
    return m_iCommand;
}

void
cobraClipUpdateEvent::setCommand(int command)
{
    m_iCommand = command;
}

cobraNetEvent*
cobraClipUpdateEvent::duplicate()
{
    cobraClipUpdateEvent* clipDupe = new cobraClipUpdateEvent(*this);
    return clipDupe;
}


cobraClipUpdateEventHandler::cobraClipUpdateEventHandler()
    :cobraNetEventHandler("ClipUpdate", cobraClipUpdateEventType)
{}

cobraClipUpdateEventHandler::cobraClipUpdateEventHandler(cobraClipUpdateEventHandler& event)
    :cobraNetEventHandler(event)
{}

cobraClipUpdateEventHandler::~cobraClipUpdateEventHandler()
{}

bool
cobraClipUpdateEventHandler::handleEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraClipUpdateEventType)
        return false;

    if (event->isRequest())
        return handleServerEvent(event);

    debug(ERROR(CRITICAL), "Shouldn't got an ClipUpdate Response...\n");
    return true;
}

bool
cobraClipUpdateEventHandler::handleServerEvent(cobraNetEvent* event)
{
//    debug(LOW, "Handling Client Authorization Request!\n");
//
//    cobraClipUpdateEvent* auth = static_cast<cobraClipUpdateEvent*>(event);
//    cobraNetHandler* netHandler = cobraNetHandler::instance();
//    cobraStateEvent* newState = new cobraStateEvent();
//
//    debug(MED, "Username: %s Password: %s\n", qPrintable(auth->username()), qPrintable(auth->password()));
//    int authorized = netHandler->isAuthorized(auth->password());
//
//    newState->setDestination(auth->source());
//    newState->setResponse(true);
//    newState->setSource(SERVER);
//
//    if (authorized) {
//        QString user = auth->username();

#if 0 /* Lets not do this for now... */
        int cnt = 1;
        while (netHandler->userExists(user)) {
            user = QString("%1%2").arg(auth->username()).arg(cnt++);
        }

        if (cnt > 1) {
            /* send a command to update the kernel name */
        }
#endif

//        debug(HIGH, "User '%s' is authorized as a %s\n", qPrintable(user), authorized&GuestAuth?"Guest":"Participant");
//
//        newState->setState(cobraStateEvent::ConnectedState);
//        netHandler->sendEvent(static_cast<cobraNetEvent*>(newState));
//
//        netHandler->setIdUsername(event->source(), user);
//        netHandler->setIdAuthorization(event->source(), authorized);
//
//        netHandler->chatNotify(SERVER, BROADCAST, QString(CHAT_NOTIFY("User '%1' has connected.\n")).arg(user));
//        netHandler->broadcastUserlist();
//
//    } else {
//        newState->setFlag(cobraStateEvent::AuthenticationFailure);
//        newState->setState(cobraStateEvent::ConnectionRefused);
//
//        cobraSendEvent(newState);
//        netHandler->removeConnection(auth->source());
//    }
//
    return true;
}

cobraNetEvent*
cobraClipUpdateEventHandler::eventGenesis() const
{
    return new cobraClipUpdateEvent();
}

