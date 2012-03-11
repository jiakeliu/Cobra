#include "cobralistwidget.h"

cobralistwidget::cobralistwidget(QWidget *parent) :
    QTreeWidget(parent)
{}

bool
cobralistwidget::updateClip(cobraClip& clip)
{
   //call parent function here to refresh database

    QString cliptitle = clip.getTitle();
    QString cliptags = clip.getTags();

    //this->clear();
    //this->addItem();
    return true;
}

bool
cobralistwidget::removeClip(int uid)
{
    return 1;
}

bool
cobralistwidget::addClip(cobraClip& clip)
{
    return 1;
}
