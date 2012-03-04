#ifndef TRANSFERSDLG_H
#define TRANSFERSDLG_H

#include <QDialog>
#include "event.h"

namespace Ui {
    class transfersdlg;
}

class cobraTransferStatistics;

class Transfersdlg : public QDialog
{
    Q_OBJECT

public:
    explicit Transfersdlg(QWidget *parent = 0);
    ~Transfersdlg();

    /**
     * @fn void setVisible(bool vis)
     * This function is used to start the update timer.
     * @param vis Set the window visible or not.
     */
    void setVisible(bool vis);

public slots:
    /**
     * @fn void updateList()
     * This function is responsible for updating the list
     * on the specified time interval.
     */
    void updateList();

protected:
    /**
     * @fn void closeEvent(QCloseEvent* event)
     * This function is intended to cancel the timer once
     * the dialog is closed.
     * @param event The close event.
     */
    void closeEvent(QCloseEvent *event);

private:
    Ui::transfersdlg *ui;

    QTimer                                  m_tUpdateTimer;
    cobraTransferStatistics*                m_ctsStats;
    cobraTransferStatistics::cobraStatMap   m_mumTransferList;
    QMap<uint32_t, QListWidgetItem*>        m_muiListMap;
};

#endif // TRANSFERSDLG_H
