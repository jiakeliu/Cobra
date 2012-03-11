#include "clip.h"
#include "debug.h"

#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QTableView>

cobraClip::cobraClip()
{
    debug(LOW, "cobraClip initializing...\n");
}

cobraClip::~cobraClip()
{
    debug(LOW, "cobraClip dieing...\n");
}

cobraClipList::cobraClipList(QString dbName)
{
    debug(LOW, "cobraClipList initializing using 1 argument, the dbName...\n");
    m_sDBName = dbName;
    m_dbDatabase = QSqlDatabase::addDatabase("QSQLITE", dbName);
    m_dbDatabase.setDatabaseName(m_sDBName);

    if (!m_dbDatabase.open()) {
        debug(ERROR(CRITICAL), "Failed to open DATABASE!\n");
        return;
    }

    QSqlQuery query(m_dbDatabase);
    
    //creates table for new db
    if (!query.exec("SELECT * FROM cobraClips"))
    {
        query.exec("CREATE TABLE  cobraClips  (uid int, path blob, hash varchar(32), size int, modtime varchar(40), title varchar(40), tags varchar(160), description blob, extension varchar(16))");
    }    
}

cobraClipList::~cobraClipList()
{
    m_dbDatabase.close();
    debug(LOW, "cobraClipList dieing...\n");
}

cobraClip cobraClipList::getClip(int uid)
{
    cobraClip ccClip;
    QSqlQuery query("SELECT " % QString::number(uid) % "FROM uid", m_dbDatabase);
    QSqlRecord sqlRecord = query.record();
    ccClip.setUID(sqlRecord.value(0).toInt());
    ccClip.setPath(sqlRecord.value(1).toString());
    ccClip.setHash(sqlRecord.value(2).toString());
    ccClip.setSize(sqlRecord.value(3).toInt());
    ccClip.setModifiedTime(sqlRecord.value(4).toString());
    ccClip.setTitle(sqlRecord.value(5).toString());
    ccClip.setTags(sqlRecord.value(6).toString());
    ccClip.setDescription(sqlRecord.value(7).toString());
    ccClip.setExtension(sqlRecord.value(8).toString());
    return ccClip;   
}

bool
cobraClipList::updateClip(cobraClip& clip)
{
    QString updateString = "UPDATE cobraClips SET path = " % 
            clip.getPath() % ", hash = " %
            clip.getHash() % ", size = " %
            QString::number(clip.getSize()) % ", modtime = " %
            clip.getModifiedTime() % ", title = " %
            clip.getTitle() % ", tags = " %
            clip.getTags() % ", description = " %
            clip.getDescription() % " WHERE uid = " %
            clip.getExtension() % " WHERE extension = " %
            QString::number(clip.getUID());


    return sqlQuery(updateString);
}

bool
cobraClipList::removeClip(int uid)
{
    QString deleteString = "DELETE FROM cobraClips WHERE "  % QString::number(uid);
    return sqlQuery(deleteString);
}

bool
cobraClipList::addClip(cobraClip& clip)
{
    QString insertString = "INSERT INTO cobraClips VALUES(" %  
            QString::number(clip.getUID()) % ", '" %
            clip.getPath() % "', '" %
            clip.getHash() % "', " %
            QString::number(clip.getSize()) % ", '" %
            clip.getModifiedTime() % "', '" %
            clip.getTitle() % "', '" %
            clip.getTags() % "', '" %
            clip.getDescription() % "')";
            clip.getExtension() % "')";

    return sqlQuery(insertString);
}

bool
cobraClipList::sqlQuery(QString& string)
{
    qDebug() << "SQL Query: " << string;
    QSqlQuery query(m_dbDatabase);

    if (!query.exec(string)) {
        debug(ERROR(CRITICAL), "Failed to Query Database!\n");
        return false;
    }

    debug(LOW,"This is the string that was submitted for the SqlQuery: %s\n", string.data());
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

