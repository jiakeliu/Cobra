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
   //@todo call parent function here to refresh database
    //if we add another dialog to update clip, should we still implement function here?

   /* int clipid = clip.getUID();
    QString cliptitle = clip.getTitle();
    QString clipdesc = clip.getDescription();
    QString cliptime = clip.getModifiedTime();
    QString cliptags = clip.getTags();
    QTreeWidgetItem *itm = new QTreeWidgetItem(this);*/

    return 1;
}

bool
cobralistwidget::removeClip(int uid)
{
    //@todo call parent function here to remove entry in database

    //delete this->currentItem();
    // OR
    //this->takeTopLevelItem(uid);

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
