#-------------------------------------------------
#
# Project created by QtCreator 2016-08-02T13:45:46
#
#-------------------------------------------------

PRJ_FOLDER = $$PWD/../../
SRC_FOLDER = $$PRJ_FOLDER/src
DFM_DEAMON_SRC_FOLDER = $$SRC_FOLDER/dde-file-manager-daemon
LIB_DFM_SRC_FOLDER = $$SRC_FOLDER/dde-file-manager-lib

QT       += core dbus concurrent

QT       -= gui

TARGET = test-dde-file-manager-daemon
CONFIG   += console
CONFIG   -= app_bundle

PKGCONFIG += x11 polkit-agent-1 polkit-qt5-1 udisks2-qt5
CONFIG(release, release|debug) {
    PKGCONFIG += dtkwidget
} else {
    PKGCONFIG += dtkwidget
}
CONFIG += c++11 link_pkgconfig

#LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager   #in test sh project
LIBS += -L$$OUT_PWD/../../src/dde-file-manager-lib -ldde-file-manager  #in pro project

CONFIG(debug, debug|release) {
    DEPENDPATH += $$LIB_DFM_SRC_FOLDER
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../dde-file-manager-lib
}

TEMPLATE = app

#include(../utils/utils.pri)
#include(../fileoperations/fileoperations.pri)

INCLUDEPATH += $$DFM_DEAMON_SRC_FOLDER/dbusservice
INCLUDEPATH += $$DFM_DEAMON_SRC_FOLDER  \
               $$LIB_DFM_SRC_FOLDER  \
               $$SRC_FOLDER/utils \
               $$LIB_DFM_SRC_FOLDER/interfaces \
               $$LIB_DFM_SRC_FOLDER/shutil

DEFINES += QT_MESSAGELOGCONTEXT

include($$DFM_DEAMON_SRC_FOLDER/src.pri)

target.path = /usr/bin

poliktpolicy.path = /usr/share/polkit-1/actions
poliktpolicy.files = dbusservice/com.deepin.filemanager.daemon.policy

systembusconf.path = /etc/dbus-1/system.d
systembusconf.files = dbusservice/com.deepin.filemanager.daemon.conf

services.path = /usr/share/dbus-1/system-services
services.files = dbusservice/com.deepin.filemanager.daemon.service

systemd_service.files = dbusservice/dde-filemanager-daemon.service
systemd_service.path = /usr/lib/systemd/system

INSTALLS += target poliktpolicy systembusconf services systemd_service

DISTFILES += \
    dbusservice/fileoperation.xml \
    dbusservice/renamejob.xml

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

#include(../../3rdparty/googletest/gtest_dependency.pri)
include(test.pri)

unix {
    LIBS += -lgtest -lgmock
}
