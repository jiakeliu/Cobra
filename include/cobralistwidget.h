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


signals:

public slots:

protected:
};

#endif // COBRALISTWIDGET_H
