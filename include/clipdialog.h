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
    void setVisible(bool vis);

private slots:
    void on_clipSelection_valueChanged(int );
    void on_titleEdit_textChanged(QString );
    void on_descEdit_textChanged(QString );
    void on_tagsEdit_textChanged(QString );
    void on_saveBtn_clicked();
    void on_revertBtn_clicked();
    void on_cancelBtn_clicked();

protected:
    void updateButtons(bool edits);
    void updateClip();

protected:
    Ui::cobraClipDialog *ui;

    cobraClipList*  m_cclList;
    QVector<int>    m_iClips;
    cobraClip       m_ccCurrent;

    bool            m_bHasEdits;

};

#endif // CLIPDIALOG_H
