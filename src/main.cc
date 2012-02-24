#include <QApplication>
#include <QTranslator>

#include "debug.h"
#include "mainwindow.h"
#include <QtNetwork/QSslSocket>
#include "net.h"

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
        if (lower.compare("low")) {
            level = LOW;
        } else if (lower.compare("med")) {
            level = MED;
        } else if (lower.compare("high")) {
            level = HIGH;
        } else if (lower.compare("crit")) {
            level = CRITICAL;
        } else if (lower.compare("err")) {
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
