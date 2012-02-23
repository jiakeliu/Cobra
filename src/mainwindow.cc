#include "mainwindow.h"
#include "ui_mainwindow_docks.h"

#include "debug.h"
#include "net.h"
#include "filevalidator.h"
#include "uperms.h"
#include "preferences.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cobraNetHandler::instance()->init();

    installEventFilter(cobraNetHandler::instance());

    QRegExp unameRexp("[a-zA-Z0-9_]+");
    ui->chatUsername->setValidator(new QRegExpValidator(unameRexp, this));

    tabifyDockWidget(ui->serverList, ui->fileList);
    tabifyDockWidget(ui->fileList, ui->cueList);

    QString style =
            "QDockWidget::title { "
            "border-top-left-radius: 0px; "
            "border-top-right-radius: 0px; "
            "border-bottom-left-radius: 0px; "
            "border-bottom-right-radius: 0px; "
            "border: 1px solid #202020; "
            "color: rgb(128,128,128); "
            "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:0.4,"
                    "stop:0.856444 rgb(115,115,115), stop:0.3 rgb(100,100,100), stop:0.980424 rgb(92,92,92)); "
            "text-align: center; "
            "padding-left: 5px; "
            "}";

    setStyleSheet(style);
    QList<QDockWidget*> docks = this->findChildren<QDockWidget*>();

    for (int x=0; x<docks.count(); x++) {
        docks.at(x)->setStyleSheet(style);
    }

    ui->sendText->installEventFilter(this);

    cobraStateEventHandler* stateHandler = new cobraStateEventHandler();
    cobraChatEventHandler* chatHandler = new cobraChatEventHandler();
    cobraAuthEventHandler* authHandler = new cobraAuthEventHandler();
    cobraTransferEventHandler* xferHandler = new cobraTransferEventHandler();
    chatHandler->setChatDock(ui->chatDock);

    cobraNetHandler::instance()->registerEventHandler(stateHandler);
    cobraNetHandler::instance()->registerEventHandler(chatHandler);
    cobraNetHandler::instance()->registerEventHandler(authHandler);
    cobraNetHandler::instance()->registerEventHandler(xferHandler);

    registerMetaCommand(&MainWindow::setName, "\\name");
    registerMetaCommand(&MainWindow::metaConnect, "\\connect");
    registerMetaCommand(&MainWindow::setAway, "\\away");

    QObject::connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendChat()));

    stateHandler->put();
    chatHandler->put();
    authHandler->put();

    /* Hide the central widget so that the dock widgets take over. */
    ui->centralwidget->hide();

    UserPermissions testUser("test");
    cobraNetHandler::instance()->addAuth(testUser);
}

bool
MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    QKeyEvent* key = NULL;

    if (!obj || !event)
        return QMainWindow::eventFilter(obj, event);

    if (event->type() != QEvent::KeyPress)
        goto out;

    key = static_cast<QKeyEvent*>(event);
    if (key->key() != Qt::Key_Enter && key->key() != Qt::Key_Return)
        goto out;

    key->ignore();

    if (sendChat())
        return true;

out:
    return QMainWindow::eventFilter(obj, event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool
MainWindow::setName(QString name)
{
    QRegExp exp("\\s+");

    QStringList cmdArgs = name.split(exp);
    if (cmdArgs.count() < 1)
        return false;

    QRegExp unameRexp("[a-zA-Z0-9_]+");
    if (!unameRexp.exactMatch(cmdArgs[0])) {
        ui->chatText->append(CHAT_NOTIFY("Failed to set name ('A-Z', 'a-z', '0-9', and '_' only!)"));
        return false;
    }

    m_cUsername = cmdArgs[0];

    ui->chatUsername->setText(m_cUsername);
    ui->chatUsername->update();
    return true;
}

bool
MainWindow::metaConnect(QString cmd)
{
    QRegExp exp("\\s+");

    QStringList cmdArgs = cmd.split(exp);
    if (cmdArgs.count() < 4)
        return false;

    cobraNetHandler::instance()->connect(cmdArgs[0], cmdArgs[1].toInt(), cmdArgs[2], cmdArgs[3]);
    return true;
}

bool
MainWindow::setAway(QString msg)
{
    debug(LOW, "Settings Away MSG: %s\n", qPrintable(msg));
    ui->chatText->append(CHAT_NOTIFY("Settings away:"));
    ui->chatText->append(msg);
    return true;
}


bool
MainWindow::sendChat()
{
    cobraChatEvent* chat = NULL;
    QString text;

    chat = new cobraChatEvent();
    if (!chat)
        return false;

    text = ui->sendText->toPlainText();
    text.remove('\n');

    if (text.isEmpty()) {
        ui->sendText->setText("");
        return false;
    }

    if (text[0] == '\\') {
        ui->sendText->setText("");
        return processMetaCommand(text);
    }

    chat->setDestination(SERVER);
    chat->setSource(cobraMyId);

    chat->setName(m_cUsername);
    chat->setMsg(text);
    cobraSendEvent(chat);

    ui->sendText->setText("");
    return true;
}

bool
MainWindow::registerMetaCommand(metaCmdEntry func, QString cmd)
{
    if (m_msfMetaMap[cmd] != NULL) {
        debug(ERROR(LOW), "Command '%s' already registered.\n", qPrintable(cmd));
        return false;
    }

    m_msfMetaMap[cmd] = func;
    debug(LOW, "Command set.\n");
    return true;
}

bool
MainWindow::processMetaCommand(QString cmd)
{
    QString args;
    QRegExp exp;
    int pos = 0;

    exp.setPattern("\\s");
    pos = cmd.indexOf(exp);
    if (pos == -1)
        goto check;

    args = cmd.right(cmd.length() - pos);
    cmd = cmd.left(pos);

    exp.setPattern("\\S");
    pos = args.indexOf(exp);
    if (pos != -1)
        args = args.right(args.length() - (pos));

    debug(LOW, "Command (%d): '%s' Args: '%s'\n", pos, qPrintable(cmd), qPrintable(args));

check:
    if (m_msfMetaMap[cmd] == NULL) {
        debug(ERROR(LOW), "Failed to find command: %s!\n", qPrintable(cmd));
        return false;
    }

    debug(LOW, "Found command!\n");
    metaCmdEntry fn = m_msfMetaMap[cmd];
    return (this->*fn)(args);
}

void
MainWindow::on_actionConnect_triggered()
{
}

void
MainWindow::on_clientButton_clicked()
{
    cobraNetHandler* cnd = cobraNetHandler::instance();

    if (!cnd->loadClientCertificates()) {
        QMessageBox::warning(this, "Certificates Not Specified!", "Before you can connect to a server, you must select the CA Certificate to use with the connection! (Preferences->Certificates)");
        return;
    }

    if (ui->ignoreHostname->isChecked()) {
        QList<QSslError> exclude;
        cobraNetHandler::instance()->getAllowedErrors(exclude);

        exclude.append(QSslError(QSslError::HostNameMismatch, cobraNetHandler::instance()->getCaCertificate()));

        cobraNetHandler::instance()->setAllowedErrors(exclude);
    }

    bool result = cnd->connect(ui->clientAddress->text(), ui->clientPort->text().toInt(), ui->clientUsername->text(), ui->clientPassword->text());
    debug(CRITICAL, "Connect: %s\n", result ? "Connection Successful!" : "Failed to Connect!");

    ui->chatUsername->setText(ui->clientUsername->text());

    //setConnectState(true);
}

void
MainWindow::on_serverStart_clicked()
{
    cobraNetHandler* cnd = cobraNetHandler::instance();

    if (!cnd->loadServerCertificates(ui->privatePassword->text())) {
        QMessageBox::warning(this, "Certificates Not Specified!", "Before you can connect to a server, you must select the CA Certificate to use with the connection! (Preferences->Certificates)");
        return;
    }

    bool result = cnd->listen(QHostAddress::Any, ui->serverPort->text().toInt());
    debug(CRITICAL, "Listen: %s\n", result ? "Listen Successful!" : "Failed to Listen!");

    //setConnectState(true);
}

/*void
MainWindow::setConnectState(bool connected)
{
    ui->clientTab->setEnabled(!connected);
    ui->serverTab->setEnabled(!connected);
}*/

void
MainWindow::on_certificateText_textChanged(const QString &ca)
{
    //g_cobra_settings->setValue("ssl/ca", ca);
}

void
MainWindow::on_localCertificateText_textChanged(const QString &local)
{
    //g_cobra_settings->setValue("ssl/local_certificate", local);
}

void
MainWindow::on_privateKeyText_textChanged(const QString &privkey)
{
    //g_cobra_settings->setValue("ssl/private_key", privkey);
}

void
MainWindow::on_chatUsername_textChanged(const QString &username)
{
    m_cUsername = username;
}

void
MainWindow::updateUserAuthoCombo()
{
}

void
MainWindow::on_addUserBtn_clicked()
{
}

void
MainWindow::on_delUserBtn_clicked()
{
}

void MainWindow::on_actionPreferences_triggered()
{
    Preferences* pDialog = new Preferences(this);
    pDialog->show();
}
