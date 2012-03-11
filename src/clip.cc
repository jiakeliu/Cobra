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

cobraClipList::cobraClipList(QString dbName) :QSqlDatabase()
{
    debug(LOW, "cobraClipList initializing using 1 argument, the dbName...\n");
    m_sDBName = dbName;
    this->addDatabase("QSQLITE", "new");
    this->setDatabaseName(dbName);
    if (!this->open())
    {
       QMessageBox::critical(0, qApp->tr("Cannot open database"),
          qApp->tr("Unable to establish a database connection.\n"
                  "Please Make sure you entered all your data correctly\n\n"
                  "Click Cancel to exit."), QMessageBox::Cancel);
    }
    QSqlQuery query(*this);
    
    //creates table for new db
    if (!query.exec("SELECT * FROM cobraClips"))
    {
        query.exec("CREATE TABLE  cobraClips  (uid int, path blob, hash varchar(32), size int, modtime varchar(40), title varchar(40), tags varchar(160), description blob )");
    }    
}

cobraClipList::~cobraClipList()
{
    debug(LOW, "cobraClipList dieing...\n");
}

cobraClip cobraClipList::getClip(int uid)
{
    cobraClip ccClip;
    QSqlQuery query("SELECT " % QString::number(uid) % "FROM uid", *this);
    QSqlRecord sqlRecord = query.record();
    ccClip.setUID(sqlRecord.value(0).toInt());
    ccClip.setPath(sqlRecord.value(1).toString());
    ccClip.setHash(sqlRecord.value(2).toString());
    ccClip.setSize(sqlRecord.value(3).toInt());
    ccClip.setModifiedTime(sqlRecord.value(4).toString());
    ccClip.setTitle(sqlRecord.value(5).toString());
    ccClip.setTags(sqlRecord.value(6).toString());
    ccClip.setDescription(sqlRecord.value(7).toString());
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
        QString::number(clip.getUID());

    sqlQuery(updateString);
    
    return true;
}

bool
cobraClipList::removeClip(int uid)
{
    QString deleteString = "DELETE FROM cobraClips WHERE "  % QString::number(uid);

    sqlQuery(deleteString);

    return true;
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

   sqlQuery(insertString);

   return true;
}

void cobraClipList::sqlQuery(QString& string)
{
    QSqlQuery query(*this);
    if (!query.exec(string))
    {
        qDebug() << "Last Error" << query.lastError().text();
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to process your request.\n"
                 "Please Make sure you entered all your data correctly\n\n"
                 "Click Cancel to exit."), QMessageBox::Cancel);
    }
    else
    {
        debug(LOW,"This is the string that was submitted for the SqlQuery: %s\n", string.data());
    }
}

