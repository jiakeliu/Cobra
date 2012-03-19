#include "debug.h"
#include "net.h"

#include <QApplication>

#include "clip.h"

cobraClipUpdateEvent::cobraClipUpdateEvent()
    :cobraNetEvent(cobraClipUpdateEventType)
{}

cobraClipUpdateEvent::cobraClipUpdateEvent(cobraClipUpdateEvent& event)
    :cobraNetEvent(event), m_iCommand(event.m_iCommand), m_ccClip(event.m_ccClip)
{}

cobraClipUpdateEvent::~cobraClipUpdateEvent()
{}

int cobraClipUpdateEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    debug(CRITICAL, "Serializing ClipUpdate Packet! %s:%s\n",
          qPrintable(QString::number(m_ccClip.getUid())), qPrintable(m_ccClip.getPath()));

    connection << m_iCommand;
    size += sizeof(m_iCommand);

    if (m_iCommand == cobraClipUpdateEvent::RequestSync)
        return size;

    connection << QString::number(m_ccClip.getUid());
    connection << m_ccClip.getPath();
    connection << m_ccClip.getHash();
    connection << QString::number(m_ccClip.getSize());
    connection << m_ccClip.getModifiedTime();
    connection << m_ccClip.getTitle();
    connection << m_ccClip.getTags();
    connection << m_ccClip.getExtension();
    connection << m_ccClip.getDescription();

    return size + QString::number(m_ccClip.getUid()).length() +
                  m_ccClip.getPath().length() +
                  m_ccClip.getHash().length() +
                  QString::number(m_ccClip.getSize()).length() +
                  m_ccClip.getModifiedTime().length() +
                  m_ccClip.getTitle().length() +
                  m_ccClip.getTags().length() +
                  m_ccClip.getExtension().length() +
                  m_ccClip.getDescription().length();
}

int
cobraClipUpdateEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    QString tmpString;

    connection >> m_iCommand;
    size += sizeof(m_iCommand);

    if (m_iCommand == cobraClipUpdateEvent::RequestSync)
        return size;

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
    m_ccClip.setExtension(tmpString);

    connection >> tmpString;
    size += tmpString.length();
    m_ccClip.setDescription(tmpString);

    return size;
}

cobraClip
cobraClipUpdateEvent::clip() const
{
    return m_ccClip;
}

void
cobraClipUpdateEvent::setClip(const cobraClip &newClip)
{
    m_ccClip = newClip;
}

int
cobraClipUpdateEvent::command() const
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

    cobraClipUpdateEvent* cup = static_cast<cobraClipUpdateEvent*>(event);
    debug(ERROR(CRITICAL), "Handling CUP Event: Client! %d\n", cup->command());

    switch(cup->command()) {
    case cobraClipUpdateEvent::Update:
        return handleUpdate(cup, serverList());

    case cobraClipUpdateEvent::Add:
        return handleAdd(cup, serverList());

    case cobraClipUpdateEvent::BlindUpdate:
        return handleBlindUpdate(cup, serverList());

    case cobraClipUpdateEvent::FileRequest:
        debug(ERROR(CRITICAL), "Not implemented!!!");
        break;

    case cobraClipUpdateEvent::FileResponse:
        return handleUpdate(cup, localList());

    case cobraClipUpdateEvent::Remove:
        debug(ERROR(CRITICAL), "Not implemented!!!");
        break;

    default:
        debug(ERROR(CRITICAL), "Default!!!");
        break;
    }

    return true;
}

bool
cobraClipUpdateEventHandler::handleServerEvent(cobraNetEvent* event)
{

    cobraClipUpdateEvent* cup = static_cast<cobraClipUpdateEvent*>(event);

    debug(ERROR(CRITICAL), "Handling CUP Event: Server! %d\n", cup->command());

    switch(cup->command()) {
    case cobraClipUpdateEvent::Update:
        return handleUpdate(cup, serverList());

    case cobraClipUpdateEvent::Add:
        return handleAdd(cup, serverList());

    case cobraClipUpdateEvent::RequestSync:
        return handleSyncRequest(cup, serverList());

    case cobraClipUpdateEvent::FileRequest:
        return handleFileRequest(cup, serverList());

    case cobraClipUpdateEvent::FileResponse:
    case cobraClipUpdateEvent::BlindUpdate:
        debug(ERROR(CRITICAL), "Server shoudln't be ever recieve this!!!");
        break;

    case cobraClipUpdateEvent::Remove:
        debug(ERROR(CRITICAL), "Not implemented!!!");
        break;

    default:
        debug(ERROR(CRITICAL), "Default!!!");
        break;
    }

    return true;
}

bool
cobraClipUpdateEventHandler::handleBlindUpdate(cobraClipUpdateEvent* event, cobraClipList* list)
{
    if (!list)
        return false;

    cobraClip clip = event->clip();
    debug(ERROR(CRITICAL), "Recieving blind update...\n");

    if (list->containsHash(clip.getHash()))
        list->updateClip(clip);

    return list->addClip(clip);
}

bool
cobraClipUpdateEventHandler::handleUpdate(cobraClipUpdateEvent* event, cobraClipList* list)
{
    if (!list)
        return false;

    cobraClip clip = event->clip();
    debug(ERROR(CRITICAL), "Recieving update...\n");
    return list->updateClip(clip);
}

bool
cobraClipUpdateEventHandler::handleAdd(cobraClipUpdateEvent* event, cobraClipList* list)
{
    if (!list)
        return false;

    cobraClip clip = event->clip();
    debug(ERROR(CRITICAL), "Recieving add...\n");
    return list->addClip(clip);
}

bool
cobraClipUpdateEventHandler::handleFileRequest(cobraClipUpdateEvent* event, cobraClipList* list)
{
    if (!list)
        return false;

    cobraClip clip = event->clip();
    cobraClip localClip = list->getClipByHash(clip.getHash());

    cobraId source = event->source();

    if (!localClip.getUid())
        return false;

    cobraTransferFile* file = new cobraTransferFile(localClip.getPath());

    file->setSource(cobraMyId);
    file->setDestination(source);
    file->setSending(true);

    if (!file->exists())
        return false;

    return cobraNetHandler::instance()->sendFile(file);
}

bool
cobraClipUpdateEventHandler::handleSyncRequest(cobraClipUpdateEvent* inevent, cobraClipList* clist)
{
    cobraClipList* hack = clist;

    if (!hack)
        return false;

    debug(ERROR(CRITICAL), "Synchorinizing!\n");

    QVector<int> list;
    hack->enumClips(list);

    for (int x=0; x<list.size(); x++) {
        cobraClip clip;
        cobraClipUpdateEvent* event = new cobraClipUpdateEvent();

        clip = clist->getClip(list.at(x));
        event->setClip(clip);
        event->setDestination(inevent->source());
        event->setSource(inevent->destination());
        event->setResponse(true);
        event->setCommand(cobraClipUpdateEvent::BlindUpdate);

        cobraNetHandler::instance()->sendEvent(event);
    }

    return true;
}

cobraNetEvent*
cobraClipUpdateEventHandler::eventGenesis() const
{
    return new cobraClipUpdateEvent();
}

