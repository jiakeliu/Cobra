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
    ui->lineEditUser->setText(g_cobra_settings->value("client/username").toString());
    ui->lineEditPass->setText(g_cobra_settings->value("client/password").toString());
    ui->guestPwd->setText(g_cobra_settings->value("server/guest").toString());
    ui->participantPwd->setText(g_cobra_settings->value("server/participant").toString());


    QRegExp unameRexp("[a-zA-Z0-9_]+");
    ui->lineEditUser->setValidator(new QRegExpValidator(unameRexp, this));
    ui->lineEditPort->setValidator(new QIntValidator(1024, 65535, this));
    ui->lineEditPort_2->setValidator(new QIntValidator(1024, 65535, this));
    //QRegExp ipv4_RegEx(IPV4_REGEX);
    QRegExp ipv4_RegEx("(((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)([.]|$)){1,4})|(([^0-2][^0-9][^0-9])(([a-zA-Z0-9_])+[.])+)");
    //QRegExp ipv4_RegEx(HOST_NAME);
    ui->lineEditIP->setValidator(new QRegExpValidator(ipv4_RegEx, this));


    QObject::connect(cobraNetHandler::instance(), SIGNAL(rejected()), this, SLOT(restoreConnection()));
}

Preferences::~Preferences()
{
    delete ui;
}

void
Preferences::showClientTab()
{
    ui->prefTab->setCurrentIndex(1);
    show();
}

void
Preferences::setConnectState(bool connected)
{
    char* temp = "not set";
    if (connected)
        temp = "true";
    else
        temp = "false";
 
    debug(LOW, "Setting the connected state to %s\n", temp); 
    debug(HIGH, "Connecting as %s\n", qPrintable(ui->lineEditUser->text()));
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
Preferences::on_lineEditUser_textChanged(const QString &username)
{
    g_cobra_settings->setValue("client/username", username);
}

void
Preferences::on_lineEditPass_textChanged(const QString &password)
{
    g_cobra_settings->setValue("client/password", password);
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
        QMessageBox::warning(this, "Certificates Not Specified!",
                             "Before you can connect to a server, you must select\n"
                             "the CA Certificate to use with the connection!\n"
                             "(Select \"Preferences->Profile\")");
        return;
    }

    if (ui->checkBoxIgnoreErr->isChecked()) {
        QList<QSslError> exclude;
        cobraNetHandler::instance()->getAllowedErrors(exclude);

        exclude.append(QSslError(QSslError::HostNameMismatch, cobraNetHandler::instance()->getCaCertificate()));

        cobraNetHandler::instance()->setAllowedErrors(exclude);
    }

    debug(HIGH, "Connecting as %s\n", qPrintable(ui->lineEditUser->text()));

    cnd->setUsername(ui->lineEditUser->text());
    cnd->setPassword(ui->lineEditPass->text());

    bool result = cnd->connect(ui->lineEditIP->text(), ui->lineEditPort->text().toInt());
    debug(CRITICAL, "Connect: %s\n", result ? "Connection Successful!" : "Failed to Connect!");

   setConnectState(true);
}



void
Preferences::on_pushButtonStart_clicked()
{
    cobraNetHandler* cnd = cobraNetHandler::instance();

    if (!cnd->loadServerCertificates(ui->lineEditKey->text())) {
        QMessageBox::warning(this, "Certificates Not Specified!",
                             "Before you can connect to a server, you must select\n"
                             "the CA Certificate, Local Certificate, and Private Key\n"
                             "to use with the connection!\n(Select \"Preferences->Profile\")");
        return;
    }

    cnd->setGuestPassword(ui->guestPwd->text());
    cnd->setSessionPassword(ui->participantPwd->text());
    cnd->setUsername(ui->lineEditUser->text());

    debug(HIGH, "Setting Participant Passphrase: %s\n", qPrintable(ui->participantPwd->text()));
    debug(HIGH, "Setting Guest Passphrase: %s\n", qPrintable(ui->guestPwd->text()));

    bool result = cnd->listen(QHostAddress::Any, ui->lineEditPort_2->text().toInt());
    debug(CRITICAL, "Listen: %s\n", result ? "Listen Successful!" : "Failed to Listen!");

    setConnectState(true);
}

void Preferences::on_guestPwd_textChanged(const QString &pwd)
{
    g_cobra_settings->setValue("server/guest", pwd);
}

void Preferences::on_participantPwd_textChanged(const QString &pwd)
{
    g_cobra_settings->setValue("server/participant", pwd);
}

void Preferences::restoreConnection()
{
    setConnectState(false);
}
