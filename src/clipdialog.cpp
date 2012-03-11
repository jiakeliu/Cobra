#include "include/clipdialog.h"
#include "ui_clipdialog.h"

cobraClipDialog::cobraClipDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::cobraClipDialog)
{
    ui->setupUi(this);
}

cobraClipDialog::~cobraClipDialog()
{
    delete ui;
}
