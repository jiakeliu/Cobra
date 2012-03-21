#include "debug.h"
#include "net.h"

#include <QApplication>

#include "timeline.h"

cobraTimelineUpdateEvent::cobraTimelineUpdateEvent()
    :cobraNetEvent(cobraTimelineUpdateEventType)
{}

cobraTimelineUpdateEvent::cobraTimelineUpdateEvent(cobraTimelineUpdateEvent& event)
    :cobraNetEvent(event), m_iCommand(event.m_iCommand), m_ctTimeline(event.m_ctTimeline)
{}

cobraTimelineUpdateEvent::~cobraTimelineUpdateEvent()
{}

int cobraTimelineUpdateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    debug(CRITICAL, "Serializing TimelineUpdate Packet! %s:%s\n",
          qPrintable(QString::number(m_ctTimeline.getUid())), qPrintable(m_ctTimeline.getPath()));

    connection << m_iCommand;
    size += sizeof(m_iCommand);

    if (m_iCommand == cobraTimelineUpdateEvent::RequestSync)
        return size;

    connection << QString::number(m_ctTimeline.getUid());
    connection << m_ctTimeline.getTitle();
    connection << m_ctTimeline.getDescription();
    connection << m_ctTimeline.getStartTime();

    return size + QString::number(m_ctTimeline.getUid()).length() +
                  m_ctTimeline.getTitle().length() +
                  m_ctTimeline.getDescription().length() +
                  m_ctTimeline.getStartTime().length();
}

int
cobraTimelineUpdateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    QString tmpString;

    connection >> m_iCommand;
    size += sizeof(m_iCommand);

    if (m_iCommand == cobraTimelineUpdateEvent::RequestSync)
        return size;

    connection >> tmpString;
    size += tmpString.length();
    m_ctTimeline.setUid(tmpString.toInt());

    connection >> tmpString;
    size += tmpString.length();
    m_ctTimeline.setTitle(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ctTimeline.setDescription(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ctTimeline.setStartTime(tmpString);

    return size;
}

cobraTimeline
cobraTimelineUpdateEvent::timeline() const
{
    return m_ctTimeline;
}

void
cobraTimelineUpdateEvent::setTimeline(const cobraTimeline &newTimeline)
{
    m_ctTimeline = newTimeline;
}

int
cobraTimelineUpdateEvent::command() const
{
    return m_iCommand;
}

void
cobraTimelineUpdateEvent::setCommand(int command)
{
    m_iCommand = command;
}

cobraNetEvent*
cobraTimelineUpdateEvent::duplicate()
{
    cobraTimelineUpdateEvent* timelineDupe = new cobraTimelineUpdateEvent(*this);
    return timelineDupe;
}


cobraTimelineUpdateEventHandler::cobraTimelineUpdateEventHandler()
    :cobraNetEventHandler("TimelineUpdate", cobraTimelineUpdateEventType)
{}

cobraTimelineUpdateEventHandler::cobraTimelineUpdateEventHandler(cobraTimelineUpdateEventHandler& event)
    :cobraNetEventHandler(event)
{}

cobraTimelineUpdateEventHandler::~cobraTimelineUpdateEventHandler()
{}

bool
cobraTimelineUpdateEventHandler::handleEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraTimelineUpdateEventType)
        return false;

    if (event->isRequest())
        return handleServerEvent(event);

    cobraTimelineUpdateEvent* cup = static_cast<cobraTimelineUpdateEvent*>(event);
    debug(ERROR(CRITICAL), "Handling CUP Event: Client! %d\n", cup->command());

    switch(cup->command()) {
    case cobraTimelineUpdateEvent::Update:
        return handleUpdate(cup, serverList());

    case cobraTimelineUpdateEvent::Add:
        return handleAdd(cup, serverList());

    case cobraTimelineUpdateEvent::Remove:
        debug(ERROR(CRITICAL), "Not implemented!!!");
        break;

    default:
        debug(ERROR(CRITICAL), "Default!!!");
        break;
    }

    return true;
}

bool
cobraTimelineUpdateEventHandler::handleServerEvent(cobraNetEvent* event)
{

    cobraTimelineUpdateEvent* cup = static_cast<cobraTimelineUpdateEvent*>(event);

    debug(ERROR(CRITICAL), "Handling CUP Event: Server! %d\n", cup->command());

    switch(cup->command()) {
    case cobraTimelineUpdateEvent::Update:
        return handleUpdate(cup, serverList());

    case cobraTimelineUpdateEvent::Add:
        return handleAdd(cup, serverList());

    case cobraTimelineUpdateEvent::Remove:
        debug(ERROR(CRITICAL), "Not implemented!!!");
        break;

    default:
        debug(ERROR(CRITICAL), "Default!!!");
        break;
    }

    return true;
}

bool
cobraTimelineUpdateEventHandler::handleUpdate(cobraTimelineUpdateEvent* event, cobraTimelineList* list)
{
    if (!list)
        return false;

    cobraTimeline timeline = event->timeline();
    debug(ERROR(CRITICAL), "Recieving update...\n");
    return list->updateTimeline(timeline);
}

bool
cobraTimelineUpdateEventHandler::handleAdd(cobraTimelineUpdateEvent* event, cobraTimelineList* list)
{
    if (!list)
        return false;

    cobraTimeline timeline = event->timeline();
    debug(ERROR(CRITICAL), "Recieving add...\n");
    return list->addTimeline(timeline);
}


cobraNetEvent*
cobraTimelineUpdateEventHandler::eventGenesis() const
{
    return new cobraTimelineUpdateEvent();
}

