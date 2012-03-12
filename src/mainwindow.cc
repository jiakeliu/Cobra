#include "mainwindow.h"
#include "ui_mainwindow_docks.h"

#include "debug.h"
#include "net.h"
#include "filevalidator.h"
#include "preferences.h"
#include "transfersdlg.h"
#include "cobralistwidget.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pDialog(NULL), m_dTransfers(NULL)
{
    ui->setupUi(this);

    cobraNetHandler::instance()->init();

    installEventFilter(cobraNetHandler::instance());

    //tabifyDockWidget(ui->serverList, ui->fileList);
    //tabifyDockWidget(ui->fileList, ui->cueList);
    ui->serverList->setVisible(false);
    ui->cueList->setVisible(false);
    ui->fileList->setVisible(false);
    ui->fileInfoDock->setVisible(false);

    QString style =
            "QDockWidget::title { "
            "border-top-left-radius: 0px; "
            "border-top-right-radius: 0px; "
            "border-bottom-left-radius: 0px; "
            "border-bottom-right-radius: 0px; "
            "border: 1px solid #202020; "
            "color: rgb(255,255,255); "
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

    QRect rect = geometry();
    rect.setHeight(500);
    setGeometry(rect);

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
    return true;
}

bool
MainWindow::metaConnect(QString cmd)
{
    QRegExp exp("\\s+");

    QStringList cmdArgs = cmd.split(exp);
    if (cmdArgs.count() < 4)
        return false;

    cobraNetHandler* handler = cobraNetHandler::instance();

    handler->setUsername(cmdArgs[2]);
    handler->setPassword(cmdArgs[3]);

    handler->connect(cmdArgs[0], cmdArgs[1].toInt());

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

    if (!cobraNetHandler::instance()->isConnected()) {
        debug(ERROR(HIGH), "Attempted to send chat while not connected\n");
        ui->sendText->setText("");
        ui->chatText->append(CHAT_NOTIFY("Failed to send message: Not Connected."));
        return false;
    }

    text = ui->sendText->toPlainText();
    text.remove('\n');

    chat = new cobraChatEvent();
    if (!chat)
        return false;

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
MainWindow::on_actionPreferences_triggered()
{
    debug(HIGH, "Show preferences...\n");
    if (!m_pDialog)
        m_pDialog = new Preferences(this);
    m_pDialog->show();
}

void
MainWindow::on_actionConnect_triggered()
{
    debug(HIGH, "Show connection preferences...\n");
    if (!m_pDialog)
        m_pDialog = new Preferences(this);
    m_pDialog->showClientTab();
}

void
MainWindow::on_actionFile_List_toggled(bool checked)
{
    ui->fileList->setVisible(checked);
    checked = !checked;
}


void
MainWindow::on_actionClip_Cue_toggled(bool checked)
{
    ui->cueList->setVisible(checked);
    checked = !checked;
}

void
MainWindow::on_actionServer_list_toggled(bool checked)
{
    ui->serverList->setVisible(checked);
    checked = !checked;
}

void
MainWindow::on_actionChat_Window_toggled(bool checked)
{
    ui->chatDock->setVisible(checked);
    checked = !checked;
}

void
MainWindow::on_actionFile_Info_toggled(bool checked)
{
    ui->fileInfoDock->setVisible(checked);
    checked=!checked;
}

void MainWindow::on_actionOpenFile_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"),QDir::homePath(), QString::null);

    if (!path.isEmpty())
    {
        QFile file(path);
        if (!file.open(QFile::ReadOnly)) {
            return;
        }
        else {
            QFileInfo file(path);
            // Place QFile Path into the SQL Database
            // Load Tagging Dialog

            QString clipTitle;
            QString clipDesc;
            QString clipTag;

            // SQL -> setClipTitle(clipTitle) where clipPath = file.absoluteFilePath();
            // SQL -> setClipDesc(clipDesc) where clipPath = file.absoluteFilePath();
            // SQL -> setClipTag(clipTag)  where clipPath = file.absoluteFilePath();

            // addClip_toCliplist();
            // refresh_Local_Clip_List();

            qDebug() << file.absoluteFilePath();
        }
    }
}

void MainWindow::on_actionTransfers_triggered()
{
    if (!m_dTransfers)
        m_dTransfers = new Transfersdlg;
    m_dTransfers->show();
}

void MainWindow::on_actionSelectUpload_triggered()
{
    if (cobraNetHandler::instance()->isServing())
        return;

    QString path = QFileDialog::getOpenFileName(this, "Send File");
    if (!QFile::exists(path)) {
        return;
    }

    cobraTransferFile* file = new cobraTransferFile(path);
    file->setDestination(SERVER);
    file->setSource(cobraMyId);
    file->setSending(true);

    cobraTransferEvent* event = new cobraTransferEvent;
    event->fromFile(file);
    event->setResponse(false);
    event->setExtension(path);

    cobraTransferController::addPendingTransfer(file);
    cobraSendEvent(event);
}
