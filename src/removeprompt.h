#ifndef REMOVEPROMPT_H
#define REMOVEPROMPT_H

#include <QDialog>

namespace Ui {
    class removeprompt;
}

class removeprompt : public QDialog
{
    Q_OBJECT

public:
    explicit removeprompt(QWidget *parent = 0);
    ~removeprompt();

private:
    Ui::removeprompt *ui;
};

#endif // REMOVEPROMPT_H
