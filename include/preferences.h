#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
    class Preferences;
}

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

    /**
     * @fn void showClientTab()
     * Set the visible tab to be the client tab.
     */
    void showClientTab();

protected:
    void setConnectState(bool connected);

private slots:
    void on_lineEditCACert_textChanged(const QString &ca);
    void on_lineEditLocalCert_textChanged(const QString &local);
    void on_lineEditPrivateKey_textChanged(const QString &privkey);
    void on_lineEditUser_textChanged(const QString &username);
    void on_lineEditPass_textChanged(const QString &password);
    void on_tbCACert_clicked();
    void on_tbLocalCert_clicked();
    void on_tbPrivateKey_clicked();
    void on_connectButton_clicked();
    void on_pushButtonStart_clicked();

    void on_guestPwd_textChanged(const QString &arg1);

    void on_participantPwd_textChanged(const QString &arg1);

    void restoreConnection();

    void on_tbStorageDir_clicked();

private:
    Ui::Preferences *ui;

};

#endif // PREFERENCES_H
