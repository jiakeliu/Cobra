#include "cobralistwidget.h"
#include <QStandardItem>
#include "debug.h"

cobralistwidget::cobralistwidget(QWidget *parent) :
    QTreeWidget(parent), cobraClipList()
{

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(clipItemChanged(QTreeWidgetItem*, int)));
}

void
cobralistwidget::showUpload(bool t)
{
    setColumnHidden(Upload, !t);
}

void
cobralistwidget::showDownload(bool t)
{
    setColumnHidden(Download, !t);
}

void
cobralistwidget::showUid(bool t)
{
    setColumnHidden(ID, !t);
}

void
cobralistwidget::configure()
{
    /* Align the colums to the left */
    setIndentation(0);

    setColumnWidth(Upload, 30);
    setColumnWidth(Download, 30);
    setColumnWidth(ID, 10);
    setColumnWidth(Hash, 48);
    setColumnWidth(Title, 120);
    setColumnWidth(Description, 205);
    setColumnWidth(ModifiedTime, 150);
    setColumnWidth(Tags, 90);
}

bool
cobralistwidget::updateClip(cobraClip& clip)
{
    bool ret = cobraClipList::updateClip(clip);
    if (!ret) {
        debug(CRITICAL, "Ah, shiznit, clipList rejected out shit!\n");
        return ret;
    }

    /**
     * This just updates the clip in teh database --
     *
     * This should NOT be the function called when a user selected to edit a Clip's information,
     * this SHOULD be the function called AFTER the user has selected to SAVE the clip information.
     */

    QString clipid = QString::number(clip.getUid());
    QString cliphash = clip.getHash().left(4);
    QString cliptitle = clip.getTitle();
    QString clipdesc = clip.getDescription();
    QString cliptime = clip.getModifiedTime();
    QString cliptags = clip.getTags();

    QList<QTreeWidgetItem*> c = this->findItems(clipid, Qt::MatchExactly, 1);

    if(c.size() != 1) {
        debug(ERROR(HIGH), "Failed to find specified ID!\n");
        return false;
    }

    QTreeWidgetItem* itm = c.takeFirst();

    itm->setCheckState(Upload, Qt::Unchecked);
    itm->setCheckState(Download, Qt::Unchecked);
    itm->setText(ID, clipid);
    itm->setText(Hash, cliphash);
    itm->setText(Title, cliptitle);
    itm->setText(Description, clipdesc);
    itm->setText(ModifiedTime, cliptime);
    itm->setText(Tags, cliptags);

    m_bChecked = true;

    return true;
}

bool
cobralistwidget::removeClip(int uid)
{
    bool ret = cobraClipList::removeClip(uid);
    if (!ret)
        return ret;

    QList<QTreeWidgetItem*> c = this->findItems(QString::number(uid), Qt::MatchExactly, 1);

    if(c.size() != 1) {
        debug(ERROR(HIGH), "Failed to find specified ID!\n");
        return false;
    }

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
    QString clipid = QString::number(clip.getUid());
    QString cliphash = clip.getHash().left(4);
    QString clipdesc = clip.getDescription();
    QString cliptime = clip.getModifiedTime();
    QString cliptags = clip.getTags();

    itm->setCheckState(Upload, Qt::Unchecked);
    itm->setCheckState(Download, Qt::Unchecked);
    itm->setText(ID, clipid);
    itm->setText(Hash, cliphash);
    itm->setText(Title, cliptitle);
    itm->setText(Description, clipdesc);
    itm->setText(ModifiedTime, cliptime);
    itm->setText(Tags, cliptags);
    this->addTopLevelItem(itm);

    m_bChecked = true;

    return true;
}

void
cobralistwidget::getSelectedUids(QVector<int>& uids)
{
    QList<QTreeWidgetItem *> items = this->selectedItems();
    for(int i = 0; i < items.size(); i++) {
        QTreeWidgetItem *itm = items.at(i);

        if (!itm)
            continue;

        uids.append(itm->text(ID).toInt(0,10));
    }
}

void
cobralistwidget::getCheckedUids(QVector<int>& uids)
{
    QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);
    debug(LOW, "Checked Items List Size: %d\n", items.size());

    for(int i = 0; i < items.size(); i++) {
        QTreeWidgetItem *itm = items.at(i);

        if (!itm)
            continue;

        if (itm->checkState(Upload) != Qt::Checked)
            continue;

        uids.append(itm->text(ID).toInt(0,10));
    }
}

bool
cobralistwidget::syncable() const
{
    return m_bChecked;
}

void
cobralistwidget::clipItemChanged(QTreeWidgetItem* item, int index)
{
    (void)item; (void)index;
    QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);

    debug(CRITICAL, "Clip item changes\n");

    for(int i = 0; i < items.size(); i++) {
        QTreeWidgetItem *itm = items.at(i);

        if (!itm)
            continue;

        if (itm->checkState(Upload) != Qt::Checked)
            continue;

        m_bChecked = true;
    }

    emit setSyncable(syncable());
}

void
cobralistwidget::synchronized()
{
    QList<QTreeWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);

    m_bChecked = false;

    for(int i = 0; i < items.size(); i++) {
        QTreeWidgetItem *itm = items.at(i);

        if (!itm)
            continue;

        itm->setCheckState(Upload, Qt::Unchecked);
    }
}
