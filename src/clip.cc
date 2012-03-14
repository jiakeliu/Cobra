#include "clip.h"
#include "debug.h"

#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QTableView>

cobraClip::cobraClip()
    :m_iSize(0), m_iUid(0)
{
    setSize(-1);
    debug(LOW, "cobraClip initializing...\n");
}

cobraClip::~cobraClip()
{
    debug(LOW, "cobraClip dieing...\n");
}

int cobraClipList::m_iLists;

int
cobraClipList::nextList() {
   return m_iLists++;
}

cobraClipList::cobraClipList(QString dbName)
{
    debug(LOW, "cobraClipList initializing using 1 argument, the dbName...\n");
    m_sDBName = dbName;
    m_dbDatabase = QSqlDatabase::addDatabase("QSQLITE", QString("ClipList%1").arg(nextList()));
    m_dbDatabase.setDatabaseName(m_sDBName);

    if (!m_dbDatabase.open()) {
        debug(ERROR(CRITICAL), "Failed to open DATABASE!\n");
        return;
    }

    QSqlQuery query(m_dbDatabase);
    QString select = "SELECT * FROM cobraClips;";
    QString create = "CREATE TABLE  cobraClips "
            "(uid INTEGER PRIMARY KEY, path blob, hash varchar(32), "
            "size int, modtime datetime, title varchar(80), "
            "tags varchar(140), description varchar(140), "
            "extension varchar(16));";

    //creates table for new db
    if (!sqlQuery(select))
        if (!sqlQuery(create))
            debug(ERROR(CRITICAL), "Failed to create requested database!\n");
}

cobraClipList::~cobraClipList()
{
    m_dbDatabase.close();
    debug(LOW, "cobraClipList dieing...\n");
}


bool
cobraClipList::contains(int uid)
{
    //if (!m_dbDatabase.isOpen())
    //    return false;

    QSqlQuery query(m_dbDatabase);

    if (!query.exec("SELECT uid FROM cobraClips WHERE uid=" % QString::number(uid) % ";")) {
        debug(LOW, "Failed to exec..\n");
        return false;
    }

    if (!query.next()) {
        debug(LOW, "Failed to exec..\n");
        return false;
    }

    return (query.value(0).toInt() == uid);
}

bool
cobraClipList::isValid() const
{
    return true; //m_dbDatabase.isOpen();
}

cobraClip cobraClipList::getClip(int uid)
{
    cobraClip ccClip;
    QSqlQuery query(m_dbDatabase);

    bool result = query.exec("SELECT uid,path,hash,size,modtime,title,tags,description,extension "
                             "FROM cobraClips WHERE uid=" % QString::number(uid) % ";");

    if (!result || !query.next()) {
        debug(ERROR(CRITICAL), "Failed to get specified clip.\n");
        return ccClip;
    }

    ccClip.setUid(query.value(0).toInt());
    ccClip.setPath(query.value(1).toString());
    ccClip.setHash(query.value(2).toString());
    ccClip.setSize(query.value(3).toInt());
    ccClip.setModifiedTime(query.value(4).toString());
    ccClip.setTitle(query.value(5).toString());
    ccClip.setTags(query.value(6).toString());
    ccClip.setDescription(query.value(7).toString());
    ccClip.setExtension(query.value(8).toString());

    return ccClip;
}

cobraClip cobraClipList::getClipByHash(QString hash)
{
    cobraClip ccClip;
    QSqlQuery query(m_dbDatabase);

    bool result = query.exec("SELECT uid,path,hash,size,modtime,title,tags,description,extension "
                             "FROM cobraClips WHERE hash=" % hash % ";");

    if (!result || !query.next()) {
        debug(ERROR(CRITICAL), "Failed to get specified clip.\n");
        return ccClip;
    }

    ccClip.setUid(query.value(0).toInt());
    ccClip.setPath(query.value(1).toString());
    ccClip.setHash(query.value(2).toString());
    ccClip.setSize(query.value(3).toInt());
    ccClip.setModifiedTime(query.value(4).toString());
    ccClip.setTitle(query.value(5).toString());
    ccClip.setTags(query.value(6).toString());
    ccClip.setDescription(query.value(7).toString());
    ccClip.setExtension(query.value(8).toString());

    return ccClip;
}

bool
cobraClipList::updateClip(cobraClip& clip)
{
    QString updateString = "UPDATE cobraClips SET "
            "path='" % clip.getPath() % "',"
            "hash='" % clip.getHash() % "',"
            "size=" % QString::number(clip.getSize()) % ","
            "modtime=CURRENT_TIMESTAMP,"
            "title='" % clip.getTitle() % "',"
            "tags='" % clip.getTags() % "',"
            "extension='" % clip.getExtension() % "',"
            "description='" % clip.getDescription() %  "' "
            "WHERE uid=" % QString::number(clip.getUid()) % ";";

    debug(LOW, "Updated: %s\n", qPrintable(updateString));
    return sqlQuery(updateString);
}

bool
cobraClipList::removeClip(int uid)
{
    QString deleteString = "DELETE FROM cobraClips WHERE uid=" % QString::number(uid) % ";";
    return sqlQuery(deleteString);
}

bool
cobraClipList::addClip(cobraClip& clip)
{
    QSqlQuery query(m_dbDatabase);
    QString insertString =
            "INSERT INTO cobraClips (uid, path, hash, size, modtime, title, tags, description, extension) "
            "VALUES (NULL, "
            "'" % clip.getPath() % "', "
            "'" % clip.getHash() % "', "
            "'" % QString::number(clip.getSize()) % "', "
            "CURRENT_TIMESTAMP, "
            "'" % clip.getTitle() % "', "
            "'" % clip.getTags() % "', "
            "'" % clip.getDescription() % "', "
            "'" % clip.getExtension() % "');";

    debug(LOW, "Inserted: %s\n", qPrintable(insertString));
    if (!query.exec(insertString)) {
        QSqlError err = m_dbDatabase.lastError();
        debug(ERROR(CRITICAL), "Failed to Query Database (%s).\n", qPrintable(m_dbDatabase.connectionName()));
        if (!err.databaseText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.databaseText()));
        if (!err.driverText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.driverText()));
        return false;
    }

    bool ok = false;
    clip.setUid(query.lastInsertId().toInt(&ok));
    return ok;
}

bool
cobraClipList::sqlQuery(QString& string)
{
    QSqlQuery query(m_dbDatabase);

    if (!query.exec(string)) {
        QSqlError err = m_dbDatabase.lastError();
        debug(ERROR(CRITICAL), "Failed to Query Database.\n");
        if (!err.databaseText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.databaseText()));
        if (!err.driverText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.driverText()));
        return false;
    }

    debug(LOW,"This is the string that was submitted for the SqlQuery: %s\n", qPrintable(string));
    return true;
}

void
cobraClipList::enumClips(QVector<int>& vector)
{
    QSqlQuery query("SELECT uid FROM cobraClips", m_dbDatabase);

    query.exec();

    while (query.next())
        vector.append(query.value(0).toInt());

    return;
}

