#include "preferences.h"
#include "ui_preferences.h"
#include "mainwindow.h"
#include "net.h"
#include "debug.h"
#include <QtGui>



QSettings* g_cobra_settings;

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences)
{
    ui->setupUi(this);

    ui->lineEditCACert->setText(g_cobra_settings->value("ssl/ca").toString());
    ui->lineEditLocalCert->setText(g_cobra_settings->value("ssl/local_certificate").toString());
    ui->lineEditPrivateKey->setText(g_cobra_settings->value("ssl/private_key").toString());
}

Preferences::~Preferences()
{
    delete ui;
}


void
Preferences::setConnectState(bool connected)
{
   ui->Client->setEnabled(!connected);
   ui->Server->setEnabled(!connected);
}

void
Preferences::on_lineEditCACert_textChanged(const QString &ca)
{
    g_cobra_settings->setValue("ssl/ca", ca);
}

void
Preferences::on_lineEditLocalCert_textChanged(const QString &local)
{
    g_cobra_settings->setValue("ssl/local_certificate", local);
}

void
Preferences::on_lineEditPrivateKey_textChanged(const QString &privkey)
{
    g_cobra_settings->setValue("ssl/private_key", privkey);
}

void
Preferences::on_tbCACert_clicked()
{
    QString path;

    path = QFileDialog::getOpenFileName(this,
                                        "Open CA Certificate",
                                        QString::null,
                                        QString::null);
    ui->lineEditCACert->setText(path);
}

void
Preferences::on_tbLocalCert_clicked()
{
    QString path;

    path = QFileDialog::getOpenFileName(this,
                                        "Open Local Certificate",
                                        QString::null,
                                        QString::null);
    ui->lineEditLocalCert->setText(path);
}

void
Preferences::on_tbPrivateKey_clicked()
{
    QString path;

    path = QFileDialog::getOpenFileName(this,
                                        "Open Private Key",
                                        QString::null,
                                        QString::null);
    ui->lineEditPrivateKey->setText(path);
}


void
Preferences::on_connectButton_clicked()
{
    cobraNetHandler* cnd = cobraNetHandler::instance();

    if (!cnd->loadClientCertificates()) {
        QMessageBox::warning(this, "Certificates Not Specified!", "Before you can connect to a server, you must select the CA Certificate to use with the connection! (Preferences->Certificates)");
        return;
    }

    if (ui->checkBoxIgnoreErr->isChecked()) {
        QList<QSslError> exclude;
        cobraNetHandler::instance()->getAllowedErrors(exclude);

        exclude.append(QSslError(QSslError::HostNameMismatch, cobraNetHandler::instance()->getCaCertificate()));

        cobraNetHandler::instance()->setAllowedErrors(exclude);
    }

    bool result = cnd->connect(ui->lineEditIP->text(), ui->lineEditPort->text().toInt(), ui->lineEditUser->text(), ui->lineEditPass->text());
    debug(CRITICAL, "Connect: %s\n", result ? "Connection Successful!" : "Failed to Connect!");

    //ui->chatUsername->setText(ui->clientUsername->text()); <-what's diff btwn this and username?

   setConnectState(true);
}



void
Preferences::on_pushButtonStart_clicked()
{
    cobraNetHandler* cnd = cobraNetHandler::instance();

    if (!cnd->loadServerCertificates(ui->lineEditKey->text())) {
        QMessageBox::warning(this, "Certificates Not Specified!", "Before you can connect to a server, you must select the CA Certificate to use with the connection! (Preferences->Certificates)");
        return;
    }

    bool result = cnd->listen(QHostAddress::Any, ui->lineEditPort_2->text().toInt());
    debug(CRITICAL, "Listen: %s\n", result ? "Listen Successful!" : "Failed to Listen!");

    setConnectState(true);
}
