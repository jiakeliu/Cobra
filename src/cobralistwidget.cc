#include "cobralistwidget.h"
#include <QStandardItem>

cobralistwidget::cobralistwidget(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setColumnCount(6);
    this->setHeaderLabels(
    QStringList()<<"Flag"<<"ID"<<"Title"<<"Description"<<"Modified Time"<<"Tags");
}

bool
cobralistwidget::updateClip(cobraClip& clip)
{
    bool ret = cobraClipList::updateClip(clip);
    if (!ret)
        return ret;

    /**
     * This just updates the clip in teh database --
     *
     * This should NOT be the function called when a user selected to edit a Clip's information,
     * this SHOULD be the function called AFTER the user has selected to SAVE the clip information.
     */
    int clipid = clip.getUID();
    QString cliptitle = clip.getTitle();
    QString clipdesc = clip.getDescription();
    QString cliptime = clip.getModifiedTime();
    QString cliptags = clip.getTags();

    /**
      Do no create  new item here...
      I would recommend making a hidden column to containt th eclipid
      then you shoul dbe able to do something along the lines of

      QList<QTreeWidgetItem*> clip = this->findItem(QString("%1").arg(clipid), ID_COLUMN);
      at that point, fi the clipid exists, clip should contain it..
      and you can just modify the item that exists in the tree...
      */

    return true;
}

bool
cobralistwidget::removeClip(int uid)
{
    //@todo call parent function here to remove entry in database

    //delete this->currentItem();
    // OR
    //this->takeTopLevelItem(uid);

    /**
       See update clip comments for insight...
       */
    return 1;
}

bool
cobralistwidget::addClip(cobraClip& clip)
{
    //@todo call parent function here to add entry to database

    QTreeWidgetItem *itm = new QTreeWidgetItem(this);
    QString clipid = (QString)clip.getUID();
    QString cliptitle = clip.getTitle();
    QString clipdesc = clip.getDescription();
    QString cliptime = clip.getModifiedTime();
    QString cliptags = clip.getTags();

    itm->setCheckState(0, Qt::Checked);
    itm->setText(1, clipid);
    itm->setText(2, cliptitle);
    itm->setText(3, clipdesc);
    itm->setText(4, cliptime);
    itm->setText(5, cliptags);
    this->addTopLevelItem(itm);

    return 1;
}
