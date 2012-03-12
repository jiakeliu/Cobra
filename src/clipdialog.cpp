#include <QtGui>

#include "include/clipdialog.h"
#include "ui_clipdialog.h"
#include "debug.h"


cobraClipDialog::cobraClipDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::cobraClipDialog),
    m_cclList(NULL), m_bHasEdits(false)
{
    ui->setupUi(this);
}

cobraClipDialog::~cobraClipDialog()
{
    delete ui;
}

void
cobraClipDialog::setVisible(bool vis)
{
    if (vis && !m_cclList) {
        QMessageBox::critical(this, tr("No List Found!"), tr("Unable to save clip!  No list was found!"));
        QDialog::setVisible(false);
        return;
    }
    QDialog::setVisible(vis);
}

bool
cobraClipDialog::setClipList(cobraClipList* list)
{
    m_cclList = list;
    return (list != NULL);
}

bool
cobraClipDialog::setClip(int uid)
{
    m_iClips.clear();
    m_iClips.push_back(uid);

    if (m_cclList && !m_cclList->isValid()) {
        debug(ERROR(CRITICAL), "Clip List is not Initialized!\n");
        return false;
    }

    if (m_cclList && !m_cclList->contains(uid)) {
        debug(ERROR(CRITICAL), "No clip in list!\n");
        return false;
    }

    ui->clipSelection->setValue(1);
    ui->clipSelection->setMaximum(1);
    ui->clipSelection->setVisible(false);

    updateClip();

    return true;
}

bool
cobraClipDialog::setClip(QVector<int> uid)
{
    m_iClips = uid;

    ui->clipSelection->setValue(1);
    ui->clipSelection->setMaximum(uid.size());
    ui->clipSelection->setSuffix(QString(" of %1").arg(uid.size()));
    ui->clipSelection->setVisible((uid.size() == 1));

    if (m_cclList && m_cclList->isValid()) {
        for (int x=0; x<uid.size(); x++)
            if (!m_cclList->contains(uid.at(x)))
                return false;
    }

    updateClip();

    return true;
}


void
cobraClipDialog::updateClip()
{
    if (!m_cclList)
        return;

    int idx = ui->clipSelection->value()-1;
    if (idx >= m_iClips.size() || idx < 0) {
        idx = 0;
        ui->clipSelection->setValue(1);
    }

    m_ccCurrent = m_cclList->getClip(m_iClips.at(idx));

    ui->uidLabel->setText(QString::number(m_ccCurrent.getUid()));
    ui->titleEdit->setText(m_ccCurrent.getTitle());
    ui->descEdit->setText(m_ccCurrent.getDescription());
    ui->tagsEdit->setText(m_ccCurrent.getTags());
    ui->modifiedLabel->setText(m_ccCurrent.getModifiedTime());
    ui->typeLabel->setText(m_ccCurrent.getExtension());
}

void
cobraClipDialog::on_clipSelection_valueChanged(int value)
{
    (void)value;
    updateClip();
}

void
cobraClipDialog::on_titleEdit_textChanged(QString txt)
{
    m_ccCurrent.setTitle(txt);
    updateButtons(true);
}

void
cobraClipDialog::on_descEdit_textChanged(QString txt)
{
    m_ccCurrent.setDescription(txt);
    updateButtons(true);
}

void
cobraClipDialog::on_tagsEdit_textChanged(QString txt)
{
    m_ccCurrent.setTags(txt);
    updateButtons(true);
}

void
cobraClipDialog::updateButtons(bool edits)
{
    m_bHasEdits = edits;

    if (m_bHasEdits) {
        ui->clipSelection->setEnabled(false);
        ui->cancelBtn->setText("Cancel");
        ui->cancelBtn->update();
        ui->saveBtn->setEnabled(true);
        ui->revertBtn->setEnabled(true);
        return;
    }

    ui->clipSelection->setEnabled(true);
    ui->cancelBtn->setText("Close");
    ui->cancelBtn->update();
    ui->saveBtn->setEnabled(false);
    ui->revertBtn->setEnabled(false);
}

void
cobraClipDialog::on_saveBtn_clicked()
{
    if (!m_cclList) {
        QMessageBox::critical(this, tr("No List Found!"), tr("Unable to save clip!  No list was found!"));
        return;
    }

    if (m_ccCurrent.getTitle() == "<title>") {
        QMessageBox::critical(this, tr("Must Have a Title!"), tr("This clip must have a title specified!"));
        return;
    }

    m_cclList->updateClip(m_ccCurrent);
    updateButtons(false);
}

void
cobraClipDialog::on_revertBtn_clicked()
{
    updateClip();
    updateButtons(false);
}

void
cobraClipDialog::on_cancelBtn_clicked()
{
    if (m_bHasEdits)
        this->reject();
    else
        this->accept();
}
