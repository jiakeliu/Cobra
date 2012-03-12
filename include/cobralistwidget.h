#ifndef COBRALISTWIDGET_H
#define COBRALISTWIDGET_H

#include <QTreeWidget>
#include "clip.h"

class cobralistwidget : public QTreeWidget, public cobraClipList
{
    Q_OBJECT
public:
    cobralistwidget(QWidget *parent = 0);

    bool updateClip(cobraClip& clip);
    bool removeClip(int uid);
    bool addClip(cobraClip& clip);

    void getSelectedUids(QVector<int>& uids);
    void getCheckedUids(QVector<int>& uids);

    void synchronized();
    bool syncable() const;

signals:
    void setSyncable(bool cansync);

public slots:
    void clipItemChanged(QTreeWidgetItem* item, int index);

protected:

    bool    m_bChecked;
    bool    m_bEdited;
};

#endif // COBRALISTWIDGET_H
