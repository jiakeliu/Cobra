#include "debug.h"
#include "net.h"

cobraNetEvent::cobraNetEvent(int type)
    :QEvent((QEvent::Type)type), m_semRefcount(1),
     m_bHandled(false), m_bResponse(false)
{
    m_idSource = cobraNetHandler::instance()->myId();
    m_idDestination = SERVER;
}

cobraNetEvent::cobraNetEvent(cobraNetEvent& event)
    :QEvent(event), m_semRefcount(1),
      m_bHandled(false), m_bResponse(event.m_bResponse),
      m_idDestination(event.m_idDestination),
      m_idSource(event.m_idSource)
{}

cobraNetEvent::~cobraNetEvent()
{
}

int
cobraNetEvent::get(int cnt)
{
    if (m_bHandled)
        return -1;

    m_semRefcount.release(cnt);
    int ret = m_semRefcount.available();

    return ret;
}

int
cobraNetEvent::put()
{
    int ret = 0;

    if (m_bHandled)
        return -1;

    m_semRefcount.acquire();
    ret = m_semRefcount.available();

    /* If we are not being handled already by QT, delete it */
    if (!ret)
        delete this;

    return ret;
}

bool
cobraNetEvent::handled() const
{
    return m_bHandled;
}

void
cobraNetEvent::setHandled(bool set)
{
    debug(LOW, "Setting Handled: %d\n", type());
    /* Here we want to zero out the semaphore so we don't assert on
     * deletion of this object. */
    while (m_semRefcount.tryAcquire()) {/* Do Nothing! */};
    m_bHandled = set;
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

    return avail;
}

int cobraNetEventHandler::get() {
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

