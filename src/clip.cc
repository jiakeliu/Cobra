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
        query.exec("CREATE TABLE  cobraClips  (uid int, path varchar(80), hash varchar(32), modtime(40), title(40), tags(160))");
    } 
    
}

cobraClipList::~cobraClipList()
{
    debug(LOW, "cobraClipList dieing...\n");
}

cobraClip cobraClipList::getClip(int uid)
{
   cobraClip ccClip;

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
