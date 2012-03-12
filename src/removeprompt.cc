#include "removeprompt.h"
#include "ui_removeprompt.h"

removeprompt::removeprompt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::removeprompt)
{
    ui->setupUi(this);
}

removeprompt::~removeprompt()
{
    delete ui;
}
