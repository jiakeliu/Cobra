#include <QApplication>
#include <QTranslator>

#include "debug.h"
#include "mainwindow.h"
#include <QtNetwork/QSslSocket>
#include "net.h"
#include "clip.h"

/**
 * @fn void handleArgs(QApplication& app)
 *
 * Hack way of handling input args for debug runs...
 * For some reason Qt doesn't have a standard way like getopts... boo.
 */
void
handleArgs(QApplication& app)
{
    int level = LOW;
    QStringList strs = app.arguments();
    for (int x=0; x<strs.count(); x++) {
        QString lower = strs.at(x).toLower();
        if (!lower.compare("low")) {
            level = LOW;
        } else if (!lower.compare("med")) {
            level = MED;
        } else if (!lower.compare("high")) {
            level = HIGH;
        } else if (!lower.compare("crit")) {
            level = CRITICAL;
        } else if (!lower.compare("err")) {
            level = ERROR(level);
        }
    }
    set_debug_level(level);
}

int
main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    handleArgs(app);

    cobraClipList myList;

    cobraClip myClip;
    myClip.setTitle("Test");
    myClip.setDescription("Testing Clips");
    myClip.setUID(1234);
    myList.addClip(myClip);

    cobraClip myClip2;
    myClip2.setTitle("Test");
    myClip2.setDescription("Testing Clips");
    myClip2.setUID(5678);
    myList.addClip(myClip2);

    cobraClip myClip3;
    myClip3.setTitle("Test");
    myClip3.setDescription("Testing Clips");
    myClip3.setUID(9876);
    myList.addClip(myClip3);

    QVector<int> myVector;
    myList.enumClips(myVector);

    qDebug() << "My Vector";
    for (int i = 0; i<myVector.size(); i++)
    {
        qDebug() << myVector[i];
    }

    QApplication::setWindowIcon(QIcon(":images/cobra_main.png"));

    QSettings cobra_settings("./cobra.ini", QSettings::IniFormat);
    g_cobra_settings = &cobra_settings;

    QTranslator translator;
    translator.load("cobratr_en");
    app.installTranslator(&translator);


    MainWindow cobraMain;
    cobraMain.show();

    app.exec();

    return 0;
}

// vim: noai:ts=4:sw=4
