#ifndef CLIPDIALOG_H
#define CLIPDIALOG_H

#include <QDialog>

namespace Ui {
    class cobraClipDialog;
}

class cobraClipDialog : public QDialog
{
    Q_OBJECT

public:
    explicit cobraClipDialog(QWidget *parent = 0);
    ~cobraClipDialog();

private:
    Ui::cobraClipDialog *ui;
};

#endif // CLIPDIALOG_H
