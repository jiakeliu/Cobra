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

    if (m_cclList && m_cclList->isValid()) {
        return m_cclList->contains(uid);
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

void cobraClipDialog::on_buttonbox_clicked(QAbstractButton* button)
{
    if (!button)
        return;
}

void cobraClipDialog::on_clipSelection_valueChanged(int )
{

}
