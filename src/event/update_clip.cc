#include "debug.h"
#include "net.h"

#include <QApplication>

cobraClipUpdateEvent::cobraClipUpdateEvent()
    :cobraNetEvent(cobraClipUpdateEventType)
{}

cobraClipUpdateEvent::cobraClipUpdateEvent(cobraClipUpdateEvent& event)
    :cobraNetEvent(event), clip(event.clip)
{}

cobraClipUpdateEvent::~cobraClipUpdateEvent()
{}

int cobraClipUpdateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    debug(CRITICAL, "Serializing ClipUpdate Packet! %s:%s\n", qPrintable(clip));
    connection << clip;
    return size + clip.length();
}

int cobraClipUpdateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    connection >> clip;
    return size + clip.length();
}

cobraClip cobraClipUpdateEvent::getClip() const
{
    return clip;
}

void cobraClipUpdateEvent::setClip(const cobraClip &clip)
{
    clip = user;
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

    debug(MED, "Username: %s Password: %s\n", qPrintable(auth->setClip()));
//    int authorized = netHandler->isAuthorized(auth->password());

    newState->setDestination(auth->source());
    newState->setResponse(true);
    newState->setSource(SERVER);

    if (authorized) {
        QString user = auth->setClip();

#if 0 /* Lets not do this for now... */
        int cnt = 1;
        while (netHandler->userExists(user)) {
            user = QString("%1%2").arg(auth->setClip()).arg(cnt++);
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

