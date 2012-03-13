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
    m_cclFocused(NULL), m_ccdDialog(NULL)
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
    ui->serverTree->setColumnWidth(3, 205);
    ui->serverTree->setColumnWidth(4, 150);
    ui->serverTree->setColumnWidth(5, 90);
    ui->localTree->setColumnWidth(0, 55);
    ui->localTree->setColumnWidth(1, 30);
    ui->localTree->setColumnWidth(2, 120);
    ui->localTree->setColumnWidth(3, 150);
    ui->localTree->setColumnWidth(4, 150);
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
    cobraClipUpdateEventHandler* clipupdate = new cobraClipUpdateEventHandler();

    chatHandler->setChatDock(ui->chatDock);
    clipupdate->setLocalList(ui->localTree);
    clipupdate->setServerList(ui->serverTree);

    cobraNetHandler::instance()->registerEventHandler(stateHandler);
    cobraNetHandler::instance()->registerEventHandler(chatHandler);
    cobraNetHandler::instance()->registerEventHandler(authHandler);
    cobraNetHandler::instance()->registerEventHandler(xferHandler);
    cobraNetHandler::instance()->registerEventHandler(clipupdate);

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

    if (event->type() == QEvent::FocusIn) {
        m_cclFocused = dynamic_cast<cobralistwidget*>(obj);
        ui->actionRemoveClip->setEnabled(true);
        ui->actionEditClip->setEnabled(true);
    } else
    if (event->type() == QEvent::FocusOut) {
        m_cclFocused = NULL;
        ui->actionRemoveClip->setEnabled(false);
        ui->actionEditClip->setEnabled(false);

    } else {

        if (!m_cclFocused)
            return false;

        cobralistwidget* widget = dynamic_cast<cobralistwidget*>(m_cclFocused);

        if (!widget)
            return false;

        QList<QTreeWidgetItem*> selection = widget->selectedItems();

        if(selection.size() > 0) {
            ui->actionEditClip->setEnabled(true);
            ui->actionRemoveClip->setEnabled(m_cclFocused == ui->localTree);
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
    cobraClipList *list = ui->localTree;
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
    QMessageBox msgBox;
    msgBox.setText(trUtf8("Are you sure you want to remove the selected clip?"));
    msgBox.setWindowTitle(tr("Warning"));
    msgBox.setModal(true);
    QPushButton *noButton = msgBox.addButton(trUtf8("     No     "), QMessageBox::YesRole);
    QPushButton *yesButton = msgBox.addButton(trUtf8(  "    Yes   "), QMessageBox::YesRole);
    msgBox.setDefaultButton(noButton);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();

    if(msgBox.clickedButton() == noButton) {
        debug(ERROR(LOW), "Canceled clip removal\n");
        msgBox.accept();
    }
    else if(msgBox.clickedButton() == yesButton) {
        debug(ERROR(LOW), "Clip removal confirmed\n");

        QTreeWidgetItem* ci = ui->localTree->currentItem();
        if (ci!=NULL)
        {
            int uid = ci->text(1).toInt(0,10);
            ui->localTree->removeClip(uid);
        } else return;
    }
}

void
MainWindow::on_actionEditClip_triggered()
{
    cobralistwidget *clw =  dynamic_cast<cobralistwidget*>(m_cclFocused);

    if (!clw)
        return;

    int res = 0;
    QTreeWidgetItem* ci = clw->currentItem();

    if (!ci) {
        debug(ERROR(CRITICAL), "Failed to find associated list item!");
        ui->actionEditClip->setEnabled(false);
        return;
    }

    if (!m_cclFocused) {
        debug(ERROR(CRITICAL), "Failed to find associated List!");
        ui->actionEditClip->setEnabled(false);
        return;
    }

    cobraClipList *list = m_cclFocused;

    cobraClip clip = m_cclFocused->getClip(ci->text(1).toInt());
    if(!clw->updateClip(clip)){
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
    bool wantUpload = false;
    bool wantDownload = false;

    if (cobraNetHandler::instance()->isConnected() == true) {
        QMessageBox msgBox;
        msgBox.setText(trUtf8("You have selected to sync with the server.  "
                              "Please select whether you wish to upload and/or download Clip Information."));
        QAbstractButton *cancelButton = msgBox.addButton(trUtf8("     Cancel     "), QMessageBox::YesRole);
        QAbstractButton *bothButton = msgBox.addButton(trUtf8(  "    Sync Both   "), QMessageBox::YesRole);
        QAbstractButton *serverButton = msgBox.addButton(trUtf8("Download Changes"), QMessageBox::YesRole);
        QAbstractButton *localButton = msgBox.addButton(trUtf8( " Upload Changes "), QMessageBox::YesRole);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.exec();

        if(msgBox.clickedButton() == cancelButton) {
            debug(ERROR(LOW), "Canceled Sync List\n");
        }
        else if(msgBox.clickedButton() == bothButton) {
            debug(ERROR(LOW), "Sync Both Lists\n");
            wantUpload = true;
            wantDownload = true;
        }
        else if(msgBox.clickedButton() == serverButton) {
            debug(ERROR(LOW), "Sync From Server List\n");
            wantDownload = true;
        }
        else if(msgBox.clickedButton() == localButton) {
            debug(ERROR(LOW), "Sync From Local List\n");
            wantUpload = true;
        }

        if (wantUpload)
            sendLocalUpdates();

        if (wantDownload)
            refreshServerList();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Please Start or Connect to a server before attempting list sync.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
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
        event->setResponse(false);
        event->setClip(clip);
        event->setCommand(cobraClipUpdateEvent::Add);

        cobraSendEvent(event);
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
        event->setResponse(false);
        event->setClip(clip);
        event->setCommand(cobraClipUpdateEvent::Add);

        cobraSendEvent(event);

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

    ui->serverTree->synchronized();
    ui->localTree->synchronized();
}

void
MainWindow::refreshServerList()
{
    cobraClipUpdateEvent* event = new cobraClipUpdateEvent();
    event->setSource(cobraMyId);
    event->setDestination(SERVER);
    event->setCommand(cobraClipUpdateEvent::RequestSync);

    cobraClipList* hack = dynamic_cast<cobraClipList*>(ui->serverTree);

    if (!hack)
        return;

    debug(ERROR(CRITICAL), "Synchorinizing!\n");

    QVector<int> list;
    hack->enumClips(list);

    for (int x=0; x<list.size(); x++) {
        hack->removeClip(list.at(x));
    }

    cobraNetHandler::instance()->sendEvent(event);
}
