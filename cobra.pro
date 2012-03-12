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
    src/cobralistwidget.cc \
    src/event/auth.cc \
    src/preferences.cc \
    src/event/transfer.cc \
    src/event/transfer_controller.cc \
    src/transfersdlg.cc \
    src/clip.cc \
    src/clipdialog.cpp \
    src/syncprompt.cc \
    src/event/update_clip.cc \
    src/removeprompt.cc

INCLUDEPATH = include

# Directories

HEADERS += \
    include/mainwindow.h \
    include/debug.h \
    include/cobralistwidget.h \
    include/net.h \
    include/event.h \
    include/filevalidator.h \
    include/preferences.h \
    include/transfersdlg.h \
    include/clip.h \
    include/transfer.h \
    include/clipdialog.h \
    include/syncprompt.h \
    src/removeprompt.h

FORMS += \
    resources/mainwindow.ui \
    resources/mainwindow_mdi.ui \
    resources/mainwindow_docks.ui \
    resources/preferences.ui \
    resources/transfersdlg.ui \
    resources/clipdialog.ui \
    resources/syncprompt.ui \
    src/removeprompt.ui

TRANSLATIONS = cobratr_cn

RESOURCES += \
    resources/cobra.qrc
