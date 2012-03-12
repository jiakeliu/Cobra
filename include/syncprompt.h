#ifndef SYNCPROMPT_H
#define SYNCPROMPT_H

#include <QDialog>

namespace Ui {
    class syncprompt;
}

class syncprompt : public QDialog
{
    Q_OBJECT

public:
    explicit syncprompt(QWidget *parent = 0);
    ~syncprompt();

private:
    Ui::syncprompt *ui;
};

#endif // SYNCPROMPT_H
