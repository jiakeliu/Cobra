/********************************************************************************
** Form generated from reading UI file 'preferences.ui'
**
** Created: Tue Feb 21 03:55:45 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PREFERENCES_H
#define UI_PREFERENCES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Preferences
{
public:
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout;
    QTabWidget *prefTab;
    QWidget *General;
    QGroupBox *Language;
    QLabel *labelLanguage;
    QComboBox *comboBoxLang;
    QPushButton *Apply;
    QGroupBox *Certificates;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QLineEdit *lineEditCACert;
    QToolButton *tbCACert;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_7;
    QLineEdit *lineEditLocalCert;
    QToolButton *tbLocalCert;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_8;
    QLineEdit *lineEditPrivateKey;
    QToolButton *tbPrivateKey;
    QWidget *Client;
    QWidget *layoutWidget1;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QLabel *iphostname;
    QLineEdit *lineEditIP;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *lineEditPort;
    QSpacerItem *horizontalSpacer_2;
    QCheckBox *checkBoxIgnoreErr;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_3;
    QLineEdit *lineEditUser;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_4;
    QLineEdit *lineEditPass;
    QSpacerItem *horizontalSpacer_4;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *connectButton;
    QWidget *Server;
    QWidget *layoutWidget11;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label;
    QLineEdit *lineEditPort_2;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_5;
    QLineEdit *lineEditKey;
    QSpacerItem *horizontalSpacer_5;
    QHBoxLayout *horizontalLayout_9;
    QPushButton *pushButtonStart;
    QWidget *Users;
    QWidget *layoutWidget2;
    QVBoxLayout *verticalLayout_6;
    QComboBox *comboBox;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_9;
    QLineEdit *lineEdit;
    QSpacerItem *horizontalSpacer_7;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_10;
    QLineEdit *lineEdit_2;
    QSpacerItem *horizontalSpacer_6;
    QHBoxLayout *horizontalLayout_14;
    QSpacerItem *horizontalSpacer_8;
    QPushButton *pushButtonAdd;
    QPushButton *pushButtonDel;
    QSpacerItem *horizontalSpacer_9;

    void setupUi(QDialog *Preferences)
    {
        if (Preferences->objectName().isEmpty())
            Preferences->setObjectName(QString::fromUtf8("Preferences"));
        Preferences->setWindowModality(Qt::NonModal);
        Preferences->resize(480, 320);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Preferences->sizePolicy().hasHeightForWidth());
        Preferences->setSizePolicy(sizePolicy);
        Preferences->setMinimumSize(QSize(480, 320));
        Preferences->setMaximumSize(QSize(480, 320));
        Preferences->setBaseSize(QSize(0, 0));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/cobra_tico.png"), QSize(), QIcon::Normal, QIcon::Off);
        Preferences->setWindowIcon(icon);
        Preferences->setModal(true);
        verticalLayout_5 = new QVBoxLayout(Preferences);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        prefTab = new QTabWidget(Preferences);
        prefTab->setObjectName(QString::fromUtf8("prefTab"));
        General = new QWidget();
        General->setObjectName(QString::fromUtf8("General"));
        Language = new QGroupBox(General);
        Language->setObjectName(QString::fromUtf8("Language"));
        Language->setGeometry(QRect(10, 140, 441, 91));
        labelLanguage = new QLabel(Language);
        labelLanguage->setObjectName(QString::fromUtf8("labelLanguage"));
        labelLanguage->setGeometry(QRect(10, 30, 61, 21));
        comboBoxLang = new QComboBox(Language);
        comboBoxLang->setObjectName(QString::fromUtf8("comboBoxLang"));
        comboBoxLang->setGeometry(QRect(110, 30, 321, 22));
        Apply = new QPushButton(Language);
        Apply->setObjectName(QString::fromUtf8("Apply"));
        Apply->setGeometry(QRect(340, 60, 90, 23));
        Certificates = new QGroupBox(General);
        Certificates->setObjectName(QString::fromUtf8("Certificates"));
        Certificates->setGeometry(QRect(10, 10, 441, 111));
        layoutWidget = new QWidget(Certificates);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 20, 421, 80));
        verticalLayout_2 = new QVBoxLayout(layoutWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(layoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setMinimumSize(QSize(90, 0));

        horizontalLayout_6->addWidget(label_6);

        lineEditCACert = new QLineEdit(layoutWidget);
        lineEditCACert->setObjectName(QString::fromUtf8("lineEditCACert"));

        horizontalLayout_6->addWidget(lineEditCACert);

        tbCACert = new QToolButton(layoutWidget);
        tbCACert->setObjectName(QString::fromUtf8("tbCACert"));

        horizontalLayout_6->addWidget(tbCACert);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setMinimumSize(QSize(90, 0));

        horizontalLayout_10->addWidget(label_7);

        lineEditLocalCert = new QLineEdit(layoutWidget);
        lineEditLocalCert->setObjectName(QString::fromUtf8("lineEditLocalCert"));

        horizontalLayout_10->addWidget(lineEditLocalCert);

        tbLocalCert = new QToolButton(layoutWidget);
        tbLocalCert->setObjectName(QString::fromUtf8("tbLocalCert"));

        horizontalLayout_10->addWidget(tbLocalCert);


        verticalLayout_2->addLayout(horizontalLayout_10);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        label_8 = new QLabel(layoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setMinimumSize(QSize(90, 0));

        horizontalLayout_11->addWidget(label_8);

        lineEditPrivateKey = new QLineEdit(layoutWidget);
        lineEditPrivateKey->setObjectName(QString::fromUtf8("lineEditPrivateKey"));

        horizontalLayout_11->addWidget(lineEditPrivateKey);

        tbPrivateKey = new QToolButton(layoutWidget);
        tbPrivateKey->setObjectName(QString::fromUtf8("tbPrivateKey"));

        horizontalLayout_11->addWidget(tbPrivateKey);


        verticalLayout_2->addLayout(horizontalLayout_11);

        prefTab->addTab(General, QString());
        Client = new QWidget();
        Client->setObjectName(QString::fromUtf8("Client"));
        layoutWidget1 = new QWidget(Client);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(10, 10, 451, 201));
        verticalLayout_3 = new QVBoxLayout(layoutWidget1);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 10, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        iphostname = new QLabel(layoutWidget1);
        iphostname->setObjectName(QString::fromUtf8("iphostname"));
        iphostname->setMinimumSize(QSize(110, 0));
        iphostname->setMaximumSize(QSize(110, 16777215));

        horizontalLayout->addWidget(iphostname);

        lineEditIP = new QLineEdit(layoutWidget1);
        lineEditIP->setObjectName(QString::fromUtf8("lineEditIP"));

        horizontalLayout->addWidget(lineEditIP);


        verticalLayout_3->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(layoutWidget1);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(110, 0));
        label_2->setMaximumSize(QSize(110, 16777215));

        horizontalLayout_2->addWidget(label_2);

        lineEditPort = new QLineEdit(layoutWidget1);
        lineEditPort->setObjectName(QString::fromUtf8("lineEditPort"));
        sizePolicy.setHeightForWidth(lineEditPort->sizePolicy().hasHeightForWidth());
        lineEditPort->setSizePolicy(sizePolicy);
        lineEditPort->setMinimumSize(QSize(30, 0));
        lineEditPort->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_2->addWidget(lineEditPort);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        checkBoxIgnoreErr = new QCheckBox(layoutWidget1);
        checkBoxIgnoreErr->setObjectName(QString::fromUtf8("checkBoxIgnoreErr"));
        checkBoxIgnoreErr->setChecked(true);

        horizontalLayout_2->addWidget(checkBoxIgnoreErr);


        verticalLayout_3->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, -1, 20, -1);
        label_3 = new QLabel(layoutWidget1);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(110, 0));
        label_3->setMaximumSize(QSize(110, 16777215));

        horizontalLayout_3->addWidget(label_3);

        lineEditUser = new QLineEdit(layoutWidget1);
        lineEditUser->setObjectName(QString::fromUtf8("lineEditUser"));
        sizePolicy.setHeightForWidth(lineEditUser->sizePolicy().hasHeightForWidth());
        lineEditUser->setSizePolicy(sizePolicy);
        lineEditUser->setMinimumSize(QSize(130, 0));
        lineEditUser->setMaximumSize(QSize(130, 16777215));

        horizontalLayout_3->addWidget(lineEditUser);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout_3->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_4 = new QLabel(layoutWidget1);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(110, 0));
        label_4->setMaximumSize(QSize(110, 16777215));

        horizontalLayout_4->addWidget(label_4);

        lineEditPass = new QLineEdit(layoutWidget1);
        lineEditPass->setObjectName(QString::fromUtf8("lineEditPass"));
        sizePolicy.setHeightForWidth(lineEditPass->sizePolicy().hasHeightForWidth());
        lineEditPass->setSizePolicy(sizePolicy);
        lineEditPass->setMinimumSize(QSize(130, 0));
        lineEditPass->setMaximumSize(QSize(130, 16777215));
        lineEditPass->setEchoMode(QLineEdit::Password);

        horizontalLayout_4->addWidget(lineEditPass);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_4);


        verticalLayout_3->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, -1, 0, -1);
        connectButton = new QPushButton(layoutWidget1);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));
        sizePolicy.setHeightForWidth(connectButton->sizePolicy().hasHeightForWidth());
        connectButton->setSizePolicy(sizePolicy);
        connectButton->setMinimumSize(QSize(70, 0));
        connectButton->setMaximumSize(QSize(70, 16777215));

        horizontalLayout_5->addWidget(connectButton);


        verticalLayout_3->addLayout(horizontalLayout_5);

        prefTab->addTab(Client, QString());
        Server = new QWidget();
        Server->setObjectName(QString::fromUtf8("Server"));
        layoutWidget11 = new QWidget(Server);
        layoutWidget11->setObjectName(QString::fromUtf8("layoutWidget11"));
        layoutWidget11->setGeometry(QRect(90, 20, 281, 107));
        verticalLayout_4 = new QVBoxLayout(layoutWidget11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label = new QLabel(layoutWidget11);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(70, 0));
        label->setMaximumSize(QSize(70, 16777215));

        horizontalLayout_7->addWidget(label);

        lineEditPort_2 = new QLineEdit(layoutWidget11);
        lineEditPort_2->setObjectName(QString::fromUtf8("lineEditPort_2"));
        lineEditPort_2->setMinimumSize(QSize(70, 0));
        lineEditPort_2->setMaximumSize(QSize(70, 16777215));

        horizontalLayout_7->addWidget(lineEditPort_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer);


        verticalLayout_4->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        label_5 = new QLabel(layoutWidget11);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(70, 0));
        label_5->setMaximumSize(QSize(70, 16777215));

        horizontalLayout_8->addWidget(label_5);

        lineEditKey = new QLineEdit(layoutWidget11);
        lineEditKey->setObjectName(QString::fromUtf8("lineEditKey"));
        lineEditKey->setEchoMode(QLineEdit::Password);

        horizontalLayout_8->addWidget(lineEditKey);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_5);


        verticalLayout_4->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        pushButtonStart = new QPushButton(layoutWidget11);
        pushButtonStart->setObjectName(QString::fromUtf8("pushButtonStart"));
        pushButtonStart->setMinimumSize(QSize(110, 0));
        pushButtonStart->setMaximumSize(QSize(110, 16777215));

        horizontalLayout_9->addWidget(pushButtonStart);


        verticalLayout_4->addLayout(horizontalLayout_9);

        prefTab->addTab(Server, QString());
        Users = new QWidget();
        Users->setObjectName(QString::fromUtf8("Users"));
        layoutWidget2 = new QWidget(Users);
        layoutWidget2->setObjectName(QString::fromUtf8("layoutWidget2"));
        layoutWidget2->setGeometry(QRect(30, 20, 401, 181));
        verticalLayout_6 = new QVBoxLayout(layoutWidget2);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        comboBox = new QComboBox(layoutWidget2);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        verticalLayout_6->addWidget(comboBox);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label_9 = new QLabel(layoutWidget2);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setMinimumSize(QSize(70, 0));

        horizontalLayout_12->addWidget(label_9);

        lineEdit = new QLineEdit(layoutWidget2);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout_12->addWidget(lineEdit);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_12->addItem(horizontalSpacer_7);


        verticalLayout_6->addLayout(horizontalLayout_12);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        label_10 = new QLabel(layoutWidget2);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setMinimumSize(QSize(70, 0));

        horizontalLayout_13->addWidget(label_10);

        lineEdit_2 = new QLineEdit(layoutWidget2);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        lineEdit_2->setEchoMode(QLineEdit::Password);

        horizontalLayout_13->addWidget(lineEdit_2);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_13->addItem(horizontalSpacer_6);


        verticalLayout_6->addLayout(horizontalLayout_13);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer_8);

        pushButtonAdd = new QPushButton(layoutWidget2);
        pushButtonAdd->setObjectName(QString::fromUtf8("pushButtonAdd"));

        horizontalLayout_14->addWidget(pushButtonAdd);

        pushButtonDel = new QPushButton(layoutWidget2);
        pushButtonDel->setObjectName(QString::fromUtf8("pushButtonDel"));

        horizontalLayout_14->addWidget(pushButtonDel);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer_9);


        verticalLayout_6->addLayout(horizontalLayout_14);

        prefTab->addTab(Users, QString());

        verticalLayout->addWidget(prefTab);


        verticalLayout_5->addLayout(verticalLayout);


        retranslateUi(Preferences);

        prefTab->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(Preferences);
    } // setupUi

    void retranslateUi(QDialog *Preferences)
    {
        Preferences->setWindowTitle(QApplication::translate("Preferences", "Preferences", 0, QApplication::UnicodeUTF8));
        Language->setTitle(QApplication::translate("Preferences", "Language", 0, QApplication::UnicodeUTF8));
        labelLanguage->setText(QApplication::translate("Preferences", "Language:", 0, QApplication::UnicodeUTF8));
        comboBoxLang->clear();
        comboBoxLang->insertItems(0, QStringList()
         << QApplication::translate("Preferences", "English", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Preferences", "Simplified Chinese", 0, QApplication::UnicodeUTF8)
        );
        Apply->setText(QApplication::translate("Preferences", "Apply Language", 0, QApplication::UnicodeUTF8));
        Certificates->setTitle(QApplication::translate("Preferences", "Certificates", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("Preferences", "CA Certificate", 0, QApplication::UnicodeUTF8));
        tbCACert->setText(QApplication::translate("Preferences", "...", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("Preferences", "Local Certificate", 0, QApplication::UnicodeUTF8));
        tbLocalCert->setText(QApplication::translate("Preferences", "...", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("Preferences", "Private Key", 0, QApplication::UnicodeUTF8));
        tbPrivateKey->setText(QApplication::translate("Preferences", "...", 0, QApplication::UnicodeUTF8));
        prefTab->setTabText(prefTab->indexOf(General), QApplication::translate("Preferences", "General", 0, QApplication::UnicodeUTF8));
        iphostname->setText(QApplication::translate("Preferences", "Server IP/Hostname", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Preferences", "Port Number", 0, QApplication::UnicodeUTF8));
        checkBoxIgnoreErr->setText(QApplication::translate("Preferences", "Ignore Hostname Errors", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Preferences", "Username", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("Preferences", "Password", 0, QApplication::UnicodeUTF8));
        connectButton->setText(QApplication::translate("Preferences", "Connect", 0, QApplication::UnicodeUTF8));
        prefTab->setTabText(prefTab->indexOf(Client), QApplication::translate("Preferences", "Client", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Preferences", "Port Number", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("Preferences", "Private Key", 0, QApplication::UnicodeUTF8));
        pushButtonStart->setText(QApplication::translate("Preferences", "Start Server", 0, QApplication::UnicodeUTF8));
        prefTab->setTabText(prefTab->indexOf(Server), QApplication::translate("Preferences", "Server", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("Preferences", "Username", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("Preferences", "Password", 0, QApplication::UnicodeUTF8));
        pushButtonAdd->setText(QApplication::translate("Preferences", "Add", 0, QApplication::UnicodeUTF8));
        pushButtonDel->setText(QApplication::translate("Preferences", "Delete", 0, QApplication::UnicodeUTF8));
        prefTab->setTabText(prefTab->indexOf(Users), QApplication::translate("Preferences", "Users", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Preferences: public Ui_Preferences {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PREFERENCES_H
