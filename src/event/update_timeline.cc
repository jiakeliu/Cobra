#include "debug.h"
#include "net.h"

#include <QApplication>

#include "timeline.h"

cobraTimelineUpdateEvent::cobraTimelineUpdateEvent()
    :cobraNetEvent(cobraTimelineUpdateEventType)
{}

cobraTimelineUpdateEvent::cobraTimelineUpdateEvent(cobraTimelineUpdateEvent& event)
    :cobraNetEvent(event), m_iCommand(event.m_iCommand), m_ccTimeline(event.m_ccTimeline)
{}

cobraTimelineUpdateEvent::~cobraTimelineUpdateEvent()
{}

int cobraTimelineUpdateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    debug(CRITICAL, "Serializing TimelineUpdate Packet! %s:%s\n",
          qPrintable(QString::number(m_ccTimeline.getUid())), qPrintable(m_ccTimeline.getPath()));

    connection << m_iCommand;
    size += sizeof(m_iCommand);

    if (m_iCommand == cobraTimelineUpdateEvent::RequestSync)
        return size;

    connection << QString::number(m_ccTimeline.getUid());
    connection << m_ccTimeline.getPath();
    connection << m_ccTimeline.getHash();
    connection << QString::number(m_ccTimeline.getSize());
    connection << m_ccTimeline.getModifiedTime();
    connection << m_ccTimeline.getTitle();
    connection << m_ccTimeline.getTags();
    connection << m_ccTimeline.getExtension();
    connection << m_ccTimeline.getDescription();

    return size + QString::number(m_ccTimeline.getUid()).length() +
                  m_ccTimeline.getPath().length() +
                  m_ccTimeline.getHash().length() +
                  QString::number(m_ccTimeline.getSize()).length() +
                  m_ccTimeline.getModifiedTime().length() +
                  m_ccTimeline.getTitle().length() +
                  m_ccTimeline.getTags().length() +
                  m_ccTimeline.getExtension().length() +
                  m_ccTimeline.getDescription().length();
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
    m_ccTimeline.setUid(tmpString.toInt());

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setPath(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setHash(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setSize(tmpString.toInt());

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setModifiedTime(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setTitle(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setTags(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setExtension(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccTimeline.setDescription(tmpString);

    return size;
}

cobraTimeline
cobraTimelineUpdateEvent::timeline() const
{
    return m_ccTimeline;
}

void
cobraTimelineUpdateEvent::setTimeline(const cobraTimeline &newTimeline)
{
    m_ccTimeline = newTimeline;
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

    case cobraTimelineUpdateEvent::BlindUpdate:
        return handleBlindUpdate(cup, serverList());

    case cobraTimelineUpdateEvent::FileRequest:
        debug(ERROR(CRITICAL), "Not implemented!!!");
        break;

    case cobraTimelineUpdateEvent::FileResponse:
        return handleUpdate(cup, localList());

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

    case cobraTimelineUpdateEvent::RequestSync:
        return handleSyncRequest(cup, serverList());

    case cobraTimelineUpdateEvent::FileRequest:
        return handleFileRequest(cup, serverList());

    case cobraTimelineUpdateEvent::FileResponse:
    case cobraTimelineUpdateEvent::BlindUpdate:
        debug(ERROR(CRITICAL), "Server shoudln't be ever recieve this!!!");
        break;

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
cobraTimelineUpdateEventHandler::handleBlindUpdate(cobraTimelineUpdateEvent* event, cobraTimelineList* list)
{
    if (!list)
        return false;

    cobraTimeline timeline = event->timeline();
    debug(ERROR(CRITICAL), "Recieving blind update...\n");

    if (list->containsHash(timeline.getHash()))
        list->updateTimeline(timeline);

    return list->addTimeline(timeline);
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

bool
cobraTimelineUpdateEventHandler::handleFileRequest(cobraTimelineUpdateEvent* event, cobraTimelineList* list)
{
    if (!list)
        return false;

    cobraTimeline timeline = event->timeline();
    cobraTimeline localTimeline = list->getTimelineByHash(timeline.getHash());

    cobraId source = event->source();

    if (!localTimeline.getUid())
        return false;

    cobraTransferFile* file = new cobraTransferFile(localTimeline.getPath());

    file->setSource(cobraMyId);
    file->setDestination(source);
    file->setSending(true);

    if (!file->exists())
        return false;

    return cobraNetHandler::instance()->sendFile(file);
}

bool
cobraTimelineUpdateEventHandler::handleSyncRequest(cobraTimelineUpdateEvent* inevent, cobraTimelineList* clist)
{
    cobraTimelineList* hack = clist;

    if (!hack)
        return false;

    debug(ERROR(CRITICAL), "Synchorinizing!\n");

    QVector<int> list;
    hack->enumTimelines(list);

    for (int x=0; x<list.size(); x++) {
        cobraTimeline timeline;
        cobraTimelineUpdateEvent* event = new cobraTimelineUpdateEvent();

        timeline = clist->getTimeline(list.at(x));
        event->setTimeline(timeline);
        event->setDestination(inevent->source());
        event->setSource(inevent->destination());
        event->setResponse(true);
        event->setCommand(cobraTimelineUpdateEvent::BlindUpdate);

        cobraNetHandler::instance()->sendEvent(event);
    }

    return true;
}

cobraNetEvent*
cobraTimelineUpdateEventHandler::eventGenesis() const
{
    return new cobraTimelineUpdateEvent();
}

