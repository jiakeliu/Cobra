#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class Preferences;

namespace Ui {
    class MainWindow;
}

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
    bool sendChat();

    /* Meta Functions */
    void on_actionPreferences_triggered();

    void on_actionConnect_triggered();

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
    Preferences* m_pDialog;
};


#endif // MAINWINDOW_H
