#-------------------------------------------------
#
# Project created by QtCreator 2021-08-09T13:53:34
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-select-dialog-x11
TEMPLATE = app

CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui dtkcore

include($$PWD/../dde-desktop/dbus/filedialog/filedialog.pri)

INCLUDEPATH += $$PWD/../dde-file-manager-lib \
               $$PWD/../dde-file-manager-lib/interfaces \
               $$PWD/../utils \

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/release -ldde-file-manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/debug -ldde-file-manager
else:unix: LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

HEADERS += \

target.path = /usr/bin

filedialog_service_x11.path = /usr/share/dbus-1/services
filedialog_service_x11.files = $$PWD/com.deepin.filemanager.filedialog_x11.service

INSTALLS += target filedialog_service_x11
