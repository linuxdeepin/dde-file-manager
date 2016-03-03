#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

QT       += core gui widgets svg dbus x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-file-manager
TEMPLATE = app

include(./widgets/widgets.pri)
include(./dialogs/dialogs.pri)
include(./utils/utils.pri)
include(./filemonitor/filemonitor.pri)
include(./dbusinterface/dbusinterface.pri)


PKGCONFIG += x11 gtk+-2.0 xcb xcb-ewmh gsettings-qt dtkbase dtkutil dtkwidget
CONFIG += c++11 link_pkgconfig
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

# Automating generation .qm files from .ts files
# system($$PWD/desktop/translate_generation.sh)


RESOURCES += \
    skin/skin.qrc \
    skin/dialogs.qrc \
    skin/filemanager.qrc

target.path = /usr/bin/


INSTALLS += target

HEADERS += \
    filemanager/app/dbusworker.h \
    filemanager/app/define.h \
    filemanager/app/global.h \
    filemanager/app/signalmanager.h \
    filemanager/controllers/appcontroller.h \
    filemanager/controllers/copyjobcontroller.h \
    filemanager/controllers/copyjobworker.h \
    filemanager/controllers/dbuscontroller.h \
    filemanager/controllers/deletejobcontroller.h \
    filemanager/controllers/deletejobworker.h \
    filemanager/controllers/fileconflictcontroller.h \
    filemanager/controllers/movejobcontroller.h \
    filemanager/controllers/movejobworker.h \
    filemanager/controllers/renamejobcontroller.h \
    filemanager/controllers/trashjobcontroller.h \
    filemanager/app/filemanagerapp.h \
    filemanager/views/filemanagerwindow.h \
    filemanager/views/dmovablemainwindow.h \
    filemanager/views/dtitlebar.h \
    filemanager/views/dleftsidebar.h \
    filemanager/views/dtoolbar.h \
    filemanager/views/dfileview.h \
    filemanager/views/ddetailview.h \
    filemanager/views/dicontextbutton.h \
    filemanager/views/dstatebutton.h \
    filemanager/views/dcheckablebutton.h \
    filemanager/models/dfilesystemmodel.h \
    filemanager/controllers/histroycontroller.h \
    filemanager/controllers/filecontroller.h \
    filemanager/app/filesignalmanager.h \
    filemanager/views/fileitem.h \
    filemanager/views/filemenumanager.h

SOURCES += \
    filemanager/app/dbusworker.cpp \
    filemanager/controllers/appcontroller.cpp \
    filemanager/controllers/copyjobcontroller.cpp \
    filemanager/controllers/copyjobworker.cpp \
    filemanager/controllers/dbuscontroller.cpp \
    filemanager/controllers/deletejobcontroller.cpp \
    filemanager/controllers/deletejobworker.cpp \
    filemanager/controllers/fileconflictcontroller.cpp \
    filemanager/controllers/movejobcontroller.cpp \
    filemanager/controllers/movejobworker.cpp \
    filemanager/controllers/renamejobcontroller.cpp \
    filemanager/controllers/trashjobcontroller.cpp \
    filemanager/main.cpp \
    filemanager/app/filemanagerapp.cpp \
    filemanager/views/filemanagerwindow.cpp \
    filemanager/views/dmovablemainwindow.cpp \
    filemanager/views/dtitlebar.cpp \
    filemanager/views/dleftsidebar.cpp \
    filemanager/views/dtoolbar.cpp \
    filemanager/views/dfileview.cpp \
    filemanager/views/ddetailview.cpp \
    filemanager/views/dicontextbutton.cpp \
    filemanager/views/dstatebutton.cpp \
    filemanager/views/dcheckablebutton.cpp \
    filemanager/models/dfilesystemmodel.cpp \
    filemanager/controllers/histroycontroller.cpp \
    filemanager/controllers/filecontroller.cpp \
    filemanager/views/fileitem.cpp \
    filemanager/views/filemenumanager.cpp

INCLUDEPATH += filemanager/models
