#ifndef COBRALISTWIDGET_H
#define COBRALISTWIDGET_H

#include <QTreeWidget>
#include "clip.h"

class cobralistwidget : public QTreeWidget, public cobraClipList
{
    Q_OBJECT
public:
    cobralistwidget(QTreeWidget *parent = 0);

    bool updateClip(cobraClip& clip);
    bool removeClip(int uid);
    bool addClip(cobraClip& clip);


signals:

public slots:

};

#endif // COBRALISTWIDGET_H
