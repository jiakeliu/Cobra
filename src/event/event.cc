#include "debug.h"
#include "net.h"

cobraNetEvent::cobraNetEvent(int type)
    :QEvent((QEvent::Type)type), m_bResponse(false)
{
    m_idSource = cobraNetHandler::instance()->myId();
    m_idDestination = SERVER;
}

cobraNetEvent::cobraNetEvent(cobraNetEvent& event)
    :QEvent(event), m_bResponse(event.m_bResponse),
      m_idDestination(event.m_idDestination),
      m_idSource(event.m_idSource)
{}

cobraNetEvent::~cobraNetEvent()
{
}

int cobraNetEvent::serialize(QDataStream& connection)
{
    connection << type();
    connection << m_bResponse;
    connection << m_idDestination;
    connection << m_idSource;

    return (sizeof(m_bResponse) + sizeof(m_idDestination) + sizeof(m_idSource));
}

int cobraNetEvent::deserialize(QDataStream& connection)
{
    connection >> m_bResponse;
    connection >> m_idDestination;
    connection >> m_idSource;

    return (sizeof(m_bResponse) + sizeof(m_idDestination) + sizeof(m_idSource));
}

int cobraNetEventHandler::put() {
    int avail = 0;
    m_semRef.acquire();
    avail = m_semRef.available();

    if (!avail) {
        delete this;
        return 0;
    }

    //debug(CRITICAL, "put! %s %d\n", qPrintable(m_sName),m_semRef.available());
    return avail;
}

int cobraNetEventHandler::get() {
    //debug(CRITICAL, "get! %s %d\n", qPrintable(m_sName), m_semRef.available());
    m_semRef.release();
    return m_semRef.available();
}

cobraNetEventHandler::cobraNetEventHandler(QString name, int event)
    :m_sName(name),m_iType(event), m_semRef(1)
{}

cobraNetEventHandler::cobraNetEventHandler(cobraNetEventHandler& event)
    :m_sName(event.m_sName), m_iType(event.m_iType), m_semRef(1)
{}

cobraNetEventHandler::~cobraNetEventHandler()
{}

