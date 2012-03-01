#include "debug.h"
#include "net.h"

cobraNetConnection::cobraNetConnection(QObject *parent)
    :QSslSocket(parent), m_bConnected(false)
{
    memset(&m_Id, 0, sizeof(m_Id));

    QList<QSslError> exclude;
    cobraNetHandler::instance()->getAllowedErrors(exclude);
    this->ignoreSslErrors(exclude);
}

cobraNetConnection::~cobraNetConnection()
{
    debug(ERROR(CRITICAL), "Connection destroyed: %d\n", m_Id);
}

void
cobraNetConnection::setNewId()
{
    m_Id = getNewId();
}

void
cobraNetConnection::setId(cobraId id)
{
    m_Id = id;
}

cobraId cobraNetConnection::id() const {
    return m_Id;
}

bool cobraNetConnection::is(cobraId id) {
    debug(CRITICAL, "isId %d (mine: %d)\n", id, m_Id);
    return ((id == m_Id) || (id == BROADCAST && m_Id != NO_ID));
}

cobraId cobraNetConnection::currentId = COBRA_ID_START;

cobraId
cobraNetConnection::getNewId()
{
    return cobraNetConnection::currentId++;
}
