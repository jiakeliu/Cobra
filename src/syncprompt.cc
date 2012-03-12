#include "include/syncprompt.h"
#include "ui_syncprompt.h"

syncprompt::syncprompt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::syncprompt)
{
    ui->setupUi(this);
}

syncprompt::~syncprompt()
{
    delete ui;
}
