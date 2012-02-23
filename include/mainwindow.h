#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

namespace Ui {
    class MainWindow;
}

#define CHAT_NOTIFY(x) "<font color=grey>" x "</font>"

class MainWindow;
typedef bool (MainWindow::*metaCmdEntry)(QString);

typedef QMap<QString, metaCmdEntry> metaCmdMap;
typedef metaCmdMap::Iterator mceIterator;
typedef metaCmdMap::ConstIterator mceConstIterator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    /**
     * @fn virtual bool eventFilter(QObject* obj, QEvent* event)
     *
     * This function is used to remove enter key presses from the send box
     * and trigger a send event.
     */
    virtual bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void on_actionConnect_triggered();
    void on_clientButton_clicked();
    void on_serverStart_clicked();

    void on_privateKeyText_textChanged(const QString &privkey);
    void on_certificateText_textChanged(const QString &ca);
    void on_localCertificateText_textChanged(const QString &local);
    void on_chatUsername_textChanged(const QString &newName);    
    void on_addUserBtn_clicked();
    void on_delUserBtn_clicked();

    void updateUserAuthoCombo();
    bool sendChat();

    /* Meta Functions */
    void on_actionPreferences_triggered();

protected:
    bool setName(QString);
    bool metaConnect(QString);
    bool setAway(QString);

protected:
    //void setConnectState(bool connected);
    bool processChatCommand(QString cmd);

protected:
    bool registerMetaCommand(metaCmdEntry func, QString cmd);
    bool processMetaCommand(QString cmd);

protected:

    metaCmdMap m_msfMetaMap;

    Ui::MainWindow *ui;

    QString m_cUsername;
};


#endif // MAINWINDOW_H
