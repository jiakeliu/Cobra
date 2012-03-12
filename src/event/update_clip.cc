#include "debug.h"
#include "net.h"

#include <QApplication>

cobraClipUpdateEvent::cobraClipUpdateEvent()
    :cobraNetEvent(cobraClipUpdateEventType)
{}

cobraClipUpdateEvent::cobraClipUpdateEvent(cobraClipUpdateEvent& event)
    :cobraNetEvent(event), m_sUsername(event.m_sUsername),
      m_sPassword(event.m_sPassword)
{}

cobraClipUpdateEvent::~cobraClipUpdateEvent()
{}

int cobraClipUpdateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    debug(CRITICAL, "Serializing ClipUpdate Packet! %s:%s\n", qPrintable(m_sUsername), qPrintable(m_sPassword));
    connection << QString::number(clip.getUid());
    connection << clip.getPath();
    connection << clip.getHash();
    connection << QString::number(clip.getSize());
    connection << clip.getModifiedtime();
    connection << clip.getTitle();
    connection << clip.getTags();
    connection << clip.getDescription;
    return size + QString::number(clip.getUid()).length() + 
                  clip.getPath().length() +
                  clip.getHash().length() +
                  QString::number(clip.getSize()).length() +
                  clip.getModifiedtime().length +
                  clip.getTitle().length() +
                  clip.getTags().length() +
                  clip.getDescription().length();
}

int cobraClipUpdateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    QString tmpString;

    connection >> tmpString;
    size += tmpString.length();
    clip.setUid(tmpString.toInt());

    connection >> tmpString;
    size += tmpString.length();
    clip.setPath(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    clip.setHash(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    clip.setSize(tmpString.toInt());

    connection >> tmpString;
    size += tmpString.length();
    clip.setModifiedTime(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    clip.setTitle(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    clip.setTags(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    clip.setDescription(tmpString);

    return size;
}

cobraClip cobraClipUpdateEvent::username() const
{
    return m_sUsername;
}

void cobraClipUpdateEvent::setUsername(const QString &user)
{
    m_sUsername = user;
}

QString cobraClipUpdateEvent::password() const
{
    return m_sPassword;
}

void cobraClipUpdateEvent::setPassword(const QString &passwd)
{
    m_sPassword = passwd;
}

cobraNetEvent*
cobraClipUpdateEvent::duplicate()
{
    cobraClipUpdateEvent* auth = new cobraClipUpdateEvent(*this);
    return auth;
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
    debug(LOW, "Handling Client Authorization Request!\n");

    cobraClipUpdateEvent* auth = static_cast<cobraClipUpdateEvent*>(event);
    cobraNetHandler* netHandler = cobraNetHandler::instance();
    cobraStateEvent* newState = new cobraStateEvent();

    debug(MED, "Username: %s Password: %s\n", qPrintable(auth->username()), qPrintable(auth->password()));
    int authorized = netHandler->isAuthorized(auth->password());

    newState->setDestination(auth->source());
    newState->setResponse(true);
    newState->setSource(SERVER);

    if (authorized) {
        QString user = auth->username();

#if 0 /* Lets not do this for now... */
        int cnt = 1;
        while (netHandler->userExists(user)) {
            user = QString("%1%2").arg(auth->username()).arg(cnt++);
        }

        if (cnt > 1) {
            /* send a command to update the kernel name */
        }
#endif

        debug(HIGH, "User '%s' is authorized as a %s\n", qPrintable(user), authorized&GuestAuth?"Guest":"Participant");

        newState->setState(cobraStateEvent::ConnectedState);
        netHandler->sendEvent(static_cast<cobraNetEvent*>(newState));

        netHandler->setIdUsername(event->source(), user);
        netHandler->setIdAuthorization(event->source(), authorized);

        netHandler->chatNotify(SERVER, BROADCAST, QString(CHAT_NOTIFY("User '%1' has connected.\n")).arg(user));
        netHandler->broadcastUserlist();

    } else {
        newState->setFlag(cobraStateEvent::AuthenticationFailure);
        newState->setState(cobraStateEvent::ConnectionRefused);

        cobraSendEvent(newState);
        netHandler->removeConnection(auth->source());
    }

    return true;
}

cobraNetEvent*
cobraClipUpdateEventHandler::eventGenesis() const
{
    return new cobraClipUpdateEvent();
}

