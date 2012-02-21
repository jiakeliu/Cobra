######################################################################
# Automatically generated by qmake (2.01a) Wed Jan 11 21:01:37 2012
######################################################################

QT += network xml sql

SOURCES = src/main.cc \
    src/mainwindow.cc \
    src/debug.cc \
    src/net/handler.cc \
    src/net/thread.cc \
    src/net/connection.cc \
    src/event/state.cc \
    src/event/event.cc \
    src/event/chat.cc \
    src/filevalidator.cc \
    src/event/auth.cc \
    src/preferences.cc

INCLUDEPATH = include

# Directories

HEADERS += \
    include/mainwindow.h \
    include/debug.h \
    include/net.h \
    include/event.h \
    include/filevalidator.h \
    include/uperms.h \
    include/preferences.h

FORMS += \
    resources/mainwindow.ui \
    resources/mainwindow_mdi.ui \
    resources/mainwindow_docks.ui \
    resources/preferences.ui

TRANSLATIONS = cobratr_cn

RESOURCES += \
    resources/cobra.qrc



