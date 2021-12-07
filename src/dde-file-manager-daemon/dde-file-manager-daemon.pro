#-------------------------------------------------
#
# Project created by QtCreator 2016-08-02T13:45:46
#
#-------------------------------------------------

QT       += core dbus concurrent

QT       -= gui

TARGET = dde-file-manager-daemon
CONFIG   += console
CONFIG   -= app_bundle

PKGCONFIG += x11 polkit-agent-1 polkit-qt5-1 udisks2-qt5
CONFIG(release, release|debug) {
    PKGCONFIG += dtkwidget
} else {
    PKGCONFIG += dtkwidget
}
CONFIG += c++11 link_pkgconfig

LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager -lKF5Codecs

CONFIG(debug, debug|release) {
    DEPENDPATH += $$PWD/../dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../dde-file-manager-lib
}

TEMPLATE = app

#include(../utils/utils.pri)
#include(../fileoperations/fileoperations.pri)

INCLUDEPATH += dbusservice
INCLUDEPATH += $$PWD/../dde-file-manager-lib $$PWD/.. \
               $$PWD/../utils \
               $$PWD/../dde-file-manager-lib/interfaces \
               $$PWD/../dde-file-manager-lib/shutil

DEFINES += QT_MESSAGELOGCONTEXT

#安全加固
QMAKE_CXXFLAGS += -fstack-protector-all
QMAKE_LFLAGS += -z now -pie -fPIE
isEqual(ARCH, mips64) | isEqual(ARCH, mips32){
    QMAKE_LFLAGS += -z noexecstack -z relro
}

include(src.pri)
SOURCES += \
    main.cpp

target.path = /usr/bin

poliktpolicy.path = $$PREFIX/share/polkit-1/actions
poliktpolicy.files = dbusservice/com.deepin.filemanager.daemon.policy

systembusconf.path = /etc/dbus-1/system.d
systembusconf.files = dbusservice/com.deepin.filemanager.daemon.conf

services.path = $$PREFIX/share/dbus-1/system-services
services.files = dbusservice/com.deepin.filemanager.daemon.service

systemd_service.files = dbusservice/dde-filemanager-daemon.service
systemd_service.path = $$PREFIX/lib/systemd/system

INSTALLS += target poliktpolicy systembusconf services systemd_service

DISTFILES += \
    dbusservice/fileoperation.xml \
    dbusservice/renamejob.xml
