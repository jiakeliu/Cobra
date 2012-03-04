#include "include/transfersdlg.h"
#include "ui_transfersdlg.h"

#include "event.h"

Transfersdlg::Transfersdlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::transfersdlg)
{
    ui->setupUi(this);

    m_ctsStats = cobraTransferStatistics::instance();
}

Transfersdlg::~Transfersdlg()
{
    delete ui;
}

void
Transfersdlg::updateList(void)
{
    m_ctsStats->updateMap(m_mumTransferList);

    ui->listWidget->clear();

    if (!m_mumTransferList.count()) {
        ui->listWidget->addItem("No Pending or Completed File Transfers!");
        return;
    }
}

void
Transfersdlg::setVisible(bool vis)
{
    if (vis) {
        QObject::connect(&m_tUpdateTimer, SIGNAL(timeout()), this, SLOT(updateList()));
        m_tUpdateTimer.setInterval(2000);
        m_tUpdateTimer.start();

        updateList();
    }

    QWidget::setVisible(vis);
}

void
Transfersdlg::closeEvent(QCloseEvent *event)
{
    event->accept();
    m_tUpdateTimer.stop();
}
