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
    db = addDatabase("QSQLITE", "new");
    db.setDatabaseName(dbName);
    if (!db.open())
    {
       QMessageBox::critical(0, qApp->tr("Cannot open database"),
          qApp->tr("Unable to establish a database connection.\n"
                  "Please Make sure you entered all your data correctly\n\n"
                  "Click Cancel to exit."), QMessageBox::Cancel);
    }
    QSqlQuery query(db);
    
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
   QSqlQuery query("SELECT " % QString::number(uid) % "FROM uid", db);
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

}

bool
cobraClipList::removeClip(int uid)
{

}

bool
cobraClipList::addClip(cobraClip& clip)
{

}
