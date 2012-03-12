#include "mainwindow.h"
#include "ui_mainwindow_docks.h"

#include "debug.h"
#include "net.h"
#include "filevalidator.h"
#include "preferences.h"
#include "transfersdlg.h"
#include "cobralistwidget.h"
#include "clipdialog.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pDialog(NULL), m_dTransfers(NULL),
    m_ccdDialog(NULL)
{
    ui->setupUi(this);

    cobraNetHandler::instance()->init();

    installEventFilter(cobraNetHandler::instance());

    //tabifyDockWidget(ui->serverList, ui->fileList);
    //tabifyDockWidget(ui->fileList, ui->cueList);
    ui->cueList->setVisible(false);
    ui->fileInfoDock->setVisible(false);
    ui->serverTree->setColumnHidden(0, true);
    ui->serverTree->setColumnWidth(1, 30);
    ui->serverTree->setColumnWidth(2, 120);
    ui->serverTree->setColumnWidth(3, 235);
    ui->serverTree->setColumnWidth(4, 120);
    ui->serverTree->setColumnWidth(5, 90);
    ui->localTree->setColumnWidth(0, 55);
    ui->localTree->setColumnWidth(1, 30);
    ui->localTree->setColumnWidth(2, 120);
    ui->localTree->setColumnWidth(3, 180);
    ui->localTree->setColumnWidth(4, 120);
    ui->localTree->setColumnWidth(5, 90);

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
    ui->serverTree->installEventFilter(this);
    ui->localTree->installEventFilter(this);

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
    QObject::connect(cobraNetHandler::instance(), SIGNAL(connected(bool)), ui->actionSync, SLOT(setEnabled(bool)));

    stateHandler->put();
    chatHandler->put();
    authHandler->put();

    /* Hide the central widget so that the dock widgets take over. */
    ui->centralwidget->hide();
    m_cclFocused = dynamic_cast<cobralistwidget*>(ui->localTree);

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
    focusFilter(obj, event);

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

bool
MainWindow::focusFilter(QObject* obj, QEvent* event)
{
    QString objName = obj->objectName();
    if (objName != ui->localTree->objectName() &&
        objName != ui->serverTree->objectName())
        return false;

    QTreeWidget* wdt = qobject_cast<QTreeWidget*>(obj);
    if (!wdt)
        return false;

    if(event->type() == QEvent::FocusOut) {

        ui->actionRemoveClip->setEnabled(false);
        ui->actionEditClip->setEnabled(false);

    } else {
        QList<QTreeWidgetItem*> selection = wdt->selectedItems();

        if(selection.size() > 0) {
            ui->actionEditClip->setEnabled(true);
            ui->actionRemoveClip->setEnabled(true);

        } else {
            ui->actionEditClip->setEnabled(false);
            ui->actionRemoveClip->setEnabled(false);
        }
    }

    return false;
}

void
MainWindow::on_actionAddClip_triggered()
{
    int res = 0;

    if (!m_cclFocused) {
        debug(ERROR(CRITICAL), "Failed to find associated List!");
        ui->actionAddClip->setEnabled(false);
        return;
    }

    cobraClipList *list = m_cclFocused;

    QString path = QFileDialog::getOpenFileName(this, tr("Open File"),QDir::homePath(), QString::null);

    if (path.isEmpty() || path.isNull())
        return;

    QFileInfo file(path);
    QByteArray cliphash = cobraTransferFile::hashFile(path);

    cobraClip clip;
    clip.setTitle("<title>");
    clip.setPath(path);
    clip.setHash(cliphash.toHex().data());
    clip.setSize(file.size());
    clip.setExtension(file.completeSuffix());

    if (!list->addClip(clip)) {
        QMessageBox::critical(this, tr("Unable to Add Clip"), tr("Failed to add the clip!"));
        return;
    }

    if (clip.getUid() == 0) {
        QMessageBox::critical(this, tr("Unable to Add Clip"), tr("Failed to add the clip!"));
        return;
    }

    if (!m_ccdDialog)
        m_ccdDialog = new cobraClipDialog;

    m_ccdDialog->setClipList(list);
    if (!m_ccdDialog->setClip(clip.getUid())) {
        QMessageBox::critical(this, tr("Unable to Add Clip"), tr("Unable to edit specified clip!"));
        return;
    }

    m_ccdDialog->setModal(true);

    m_ccdDialog->exec();
    res = m_ccdDialog->result();

    m_ccdDialog->setClipList(NULL);

    if (res == QDialog::Rejected)
        list->removeClip(clip.getUid());
}

void
MainWindow::on_actionRemoveClip_triggered()
{
    cobralistwidget *clw =  dynamic_cast<cobralistwidget*>(m_cclFocused);

    if (!clw)
        return;

    QTreeWidgetItem* ci = clw->currentItem();
    if (ci!=NULL)
    {
        int uid = ci->text(1).toInt(0,10);
        clw->removeClip(uid);
    } else return;
}

void
MainWindow::on_actionEditClip_triggered()
{
    cobralistwidget *clw =  dynamic_cast<cobralistwidget*>(m_cclFocused);

    if (!clw)
        return;


    QTreeWidgetItem* ci = clw->currentItem();

    int res = 0;

    if (!m_cclFocused) {
        debug(ERROR(CRITICAL), "Failed to find associated List!");
        ui->actionEditClip->setEnabled(false);
        return;
    }

    cobraClipList *list = m_cclFocused;

    cobraClip clip;
    clip.setTitle(ci->text(2));
    clip.setDescription(ci->text(3));
    clip.setTags(ci->text(5));
    clip.setUid(ci->text(1).toInt(0, 10));

    if(!clw->updateClip(clip)){
        QMessageBox::critical(this, tr("Unable to Edit Clip"), tr("Failed to edit the clip!"));
        return;
    }

    if (clip.getUid() == 0) {
        QMessageBox::critical(this, tr("Unable to Edit Clip"), tr("Failed to edit the clip!"));
        return;
    }

    if (!m_ccdDialog)
        m_ccdDialog = new cobraClipDialog;

    m_ccdDialog->setClipList(list);
    if (!m_ccdDialog->setClip(clip.getUid())) {
        QMessageBox::critical(this, tr("Unable to Edit Clip"), tr("Unable to edit specified clip!"));
        return;
    }

    m_ccdDialog->setModal(true);

    m_ccdDialog->exec();
    res = m_ccdDialog->result();

    m_ccdDialog->setClipList(NULL);
}

void
MainWindow::on_actionSync_triggered()
{
    bool localFiles = ui->localTree->syncable();
    bool localEdits = ui->serverTree->syncable();
    bool wantUpload = false;
    bool wantDownload = false;

    QMessageBox msgBox;
    msgBox.setText(trUtf8("You have selected to sync with the server.  "
                          "Please select whether you wish to upload and/or download Clip Information."));
    QAbstractButton *cancelButton = msgBox.addButton(trUtf8("     Cancel     "), QMessageBox::YesRole);
    QAbstractButton *bothButton = msgBox.addButton(trUtf8(  "    Sync Both   "), QMessageBox::YesRole);
    QAbstractButton *serverButton = msgBox.addButton(trUtf8("Download Changes"), QMessageBox::YesRole);
    QAbstractButton *localButton = msgBox.addButton(trUtf8( " Upload Changes "), QMessageBox::YesRole);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.exec();

    if (!localFiles && !localEdits) {
        bothButton->setEnabled(false);
        serverButton->setEnabled(false);
    }

    if(msgBox.clickedButton() == cancelButton) {
        debug(ERROR(LOW), "Canceled Sync List\n");
    }
    else if(msgBox.clickedButton() == bothButton) {
        debug(ERROR(LOW), "Sync Both Lists\n");
        wantUpload = localEdits|localFiles;
        wantDownload = true;
    }
    else if(msgBox.clickedButton() == serverButton) {
        debug(ERROR(LOW), "Sync From Server List\n");
        wantDownload = true;
    }
    else if(msgBox.clickedButton() == localButton) {
        debug(ERROR(LOW), "Sync From Local List\n");
        wantUpload = localEdits|localFiles;
    }

    if (wantUpload)
        sendLocalUpdates();

    if (wantDownload)
        refreshServerList();
}

void
MainWindow::sendLocalUpdates()
{
    debug(LOW, "Send local updates!\n");
    QVector<int> localList;
    QVector<int> serverList;

    ui->localTree->getCheckedUids(localList);
    ui->serverTree->getCheckedUids(serverList);

    for (int x=0; x<serverList.size(); x++)
    {
        cobraClip clip = ui->serverTree->getClip(serverList.at(x));
        if (clip.getUid() != serverList.at(x)) {
            debug(ERROR(CRITICAL), "Failed to find selected server clip!");
            continue;
        }

        cobraClipUpdateEvent* event = new cobraClipUpdateEvent();
        event->setSource(cobraMyId);
        event->setDestination(SERVER);
        event->setClip(clip);
        event->setCommand(cobraClipUpdateEvent::Update);

        cobraNetHandler::instance()->sendEvent(event);
    }

    for (int x=0; x<localList.size(); x++)
    {
        debug(CRITICAL, "Processing UID: %d\n", localList.at(x));
        cobraClip clip = ui->localTree->getClip(localList.at(x));
        if (clip.getUid() != localList.at(x)) {
            debug(ERROR(CRITICAL), "Failed to find selected server clip!");
            continue;
        }

        cobraClipUpdateEvent* event = new cobraClipUpdateEvent();
        event->setSource(cobraMyId);
        event->setDestination(SERVER);
        event->setClip(clip);
        event->setCommand(cobraClipUpdateEvent::Add);

        cobraNetHandler::instance()->sendEvent(event);

        QString path = clip.getPath();
        cobraTransferFile* file = new cobraTransferFile(path);
        file->setDestination(SERVER);
        file->setSource(cobraMyId);
        file->setSending(true);

        cobraTransferEvent* tevent = new cobraTransferEvent;
        tevent->fromFile(file);
        tevent->setResponse(false);
        tevent->setExtension(path);

        cobraTransferController::addPendingTransfer(file);
        cobraSendEvent(tevent);
    }
}

void
MainWindow::refreshServerList()
{

}
