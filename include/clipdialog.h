#ifndef CLIPDIALOG_H
#define CLIPDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "clip.h"

namespace Ui {
    class cobraClipDialog;
}

class cobraClipDialog : public QDialog
{
    Q_OBJECT

public:
    explicit cobraClipDialog(QWidget *parent = 0);
    ~cobraClipDialog();

public:
    bool setClipList(cobraClipList* list);
    bool setClip(int uid);
    bool setClip(QVector<int> uid);

protected:
    void updateClip();

protected:
    Ui::cobraClipDialog *ui;

    cobraClipList*  m_cclList;
    QVector<int>    m_iClips;
    cobraClip       m_ccCurrent;

private slots:
    void on_buttonbox_clicked(QAbstractButton* button);
    void on_clipSelection_valueChanged(int );
};

#endif // CLIPDIALOG_H
