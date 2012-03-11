#include "debug.h"
#include "net.h"


char magicXform[] = {0x31, 0x33, 0x73, 0x59, 0x37, 0x23, 0x33, 0x7e};
QByteArray cobraStreamMagic(&magicXform[0], sizeof(magicXform));

cobraNetConnection::cobraNetConnection(QObject *parent)
    :QSslSocket(parent), m_bConnected(false)
{
    memset(&m_Id, 0, sizeof(m_Id));

    QList<QSslError> exclude;
    cobraNetHandler::instance()->getAllowedErrors(exclude);
    this->ignoreSslErrors(exclude);

    m_baRead.reserve(4096);
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

cobraId
cobraNetConnection::id() const {
    return m_Id;
}

int
cobraNetConnection::readEvents(QByteArray& events)
{
    m_baRead.append(this->readAll());

    int count = 0;
    uint32_t size = 0;

    qint64 pos = 0;
    qint64 npos = 1;
    qint64 dsize = m_baRead.count();
    qint64 valid_offset = 0;

    bool update_pos = true;

    uint32_t soffset = cobraStreamMagic.size() + sizeof(size);

    do {
        if (update_pos)
            pos = m_baRead.indexOf(cobraStreamMagic, pos);
        npos = m_baRead.indexOf(cobraStreamMagic, npos);

        if (npos == pos) {
            if (npos < 0) {
                /* No more events... */
                debug(LOW, "Event Queue Emptied!\n");
                break;
            }

            m_baRead.remove(0, pos);
            pos = 0;
            npos = 1;
            debug(ERROR(CRITICAL), "Fuck... Missed an event.\n");
            continue;
        }

        QByteArray mid = m_baRead.mid(pos+cobraStreamMagic.size(), sizeof(size));
        QDataStream dstream(&mid, QIODevice::ReadOnly);
        dstream.setByteOrder(QDataStream::LittleEndian);
        dstream >> size;

        debug(HIGH, "Size: %lx of %llx\n", size, dsize);

        if (npos < 0) {
            if (dsize - (cobraStreamMagic.size() + sizeof(size)) == size) {
                debug(HIGH, "One Event recieved!\n");
                valid_offset = dsize;
                count = 1;
                break;
            }
            npos = dsize;
        }

        qint64 result = npos - (pos + soffset + size);
        if (!result) {
            /* equal -- yeah... NEXT!*/
            debug(LOW, "Equal!!!\n");
            count++;
            valid_offset = npos;
            pos = npos++;
            update_pos = true;
            continue;
        } else if (result < 0) {
            /* Partial Packet! */
            break;
        }

        debug(LOW, "Uuuh, we somehow got a set of packets which had magics followed by numbers that LINED up but"
              "resulted in a mismatch on the NEXT event!\n");
        /* Serious issues, wipe it all out... */
        valid_offset = npos;
        break;
    } while (1);

    debug(LOW, "Count: %d  Offset: %d\n", count, valid_offset);
    events = m_baRead.left(valid_offset);
    m_baRead.remove(0, valid_offset);

    return count;
}

bool
cobraNetConnection::is(cobraId id) {
    debug(CRITICAL, "isId %d (mine: %d)\n", id, m_Id);
    return ((id == m_Id) || (id == BROADCAST && m_Id != NO_ID));
}

cobraId cobraNetConnection::currentId = COBRA_ID_START;

cobraId
cobraNetConnection::getNewId()
{
    return cobraNetConnection::currentId++;
}
