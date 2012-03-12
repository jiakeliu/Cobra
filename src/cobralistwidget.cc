#include "cobralistwidget.h"
#include <QStandardItem>

cobralistwidget::cobralistwidget(QWidget *parent) :
    QTreeWidget(parent)
{
    this->setColumnCount(6);
    this->setHeaderLabels(
    QStringList()<<"Flag"<<"ID"<<"Title"<<"Description"<<"Modified OMG"<<"Tags");
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

    QString clipid = (QString)clip.getUid();
    QString cliptitle = clip.getTitle();
    QString clipdesc = clip.getDescription();
    QString cliptime = clip.getModifiedTime();
    QString cliptags = clip.getTags();

    QList<QTreeWidgetItem*> c = this->findItems(clipid, Qt::MatchExactly, 1);

    if(c.size() != 1)
        return false;

    QTreeWidgetItem* itm = c.takeFirst();
    itm->setCheckState(0, Qt::Checked);
    itm->setText(1, clipid);
    itm->setText(2, cliptitle);
    itm->setText(3, clipdesc);
    itm->setText(4, cliptime);
    itm->setText(5, cliptags);


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
    bool ret = cobraClipList::removeClip(uid);
    if (!ret)
        return ret;

    QList<QTreeWidgetItem*> c = this->findItems((QString)uid, Qt::MatchExactly, 1);

    if(c.size() != 1)
        return false;

    delete c.takeFirst();

    return true;
}

bool
cobralistwidget::addClip(cobraClip& clip)
{
    bool ret = cobraClipList::addClip(clip);
    if (!ret)
        return ret;

    QTreeWidgetItem *itm = new QTreeWidgetItem(this);

    QString cliptitle = clip.getTitle();
    QString clipdesc = clip.getDescription();
    QString cliptime = clip.getModifiedTime();
    QString cliptags = clip.getTags();

    itm->setCheckState(0, Qt::Checked);
    itm->setText(1, QString::number(clip.getUid()));
    itm->setText(2, cliptitle);
    itm->setText(3, clipdesc);
    itm->setText(4, cliptime);
    itm->setText(5, cliptags);
    this->addTopLevelItem(itm);

    return true;
}

void
cobralistwidget::getSelectedUids(QVector<int>& uids)
{
    QList<QTreeWidgetItem *> items = this->selectedItems();
    for(int i = 0; i < items.size(); i++)
    {
        QTreeWidgetItem *itm = items.at(i);
        uids.append(itm->text(1).toInt(0,10));
    }
}
