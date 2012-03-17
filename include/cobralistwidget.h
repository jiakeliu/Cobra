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

    void showUpload(bool t);
    void showDownload(bool t);
    void showUid(bool t);

    void configure();

    enum cobraCondition {
        Update = 0x1, Add = 0x2
    };
    void setAutoCheck(int item, int condition);

signals:
    void setSyncable(bool cansync);

public slots:
    void clipItemChanged(QTreeWidgetItem* item, int index);

public:
    enum ClipColumns{
        Upload, Download, ID, Hash, Title, Description, Tags, ModifiedTime
    };

protected:

    bool    m_bChecked;
};

#endif // COBRALISTWIDGET_H
