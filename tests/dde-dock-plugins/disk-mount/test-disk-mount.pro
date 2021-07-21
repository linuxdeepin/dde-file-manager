PREFIX = /usr
QT              += core widgets concurrent dbus testlib
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkwidget gio-unix-2.0 gio-qt udisks2-qt5

SRC_FOLDER = $$PWD/../../../src

INCLUDEPATH += /usr/include/dde-dock
INCLUDEPATH += $$SRC_FOLDER/dde-file-manager-lib/interfaces \
               $$SRC_FOLDER/dde-file-manager-lib


TARGET = test-dde-disk-mount-plugin
TEMPLATE = app

DISTFILES       += disk-mount.json

DEFINES += QT_MESSAGELOGCONTEXT DFM_NO_FILE_WATCHER

include($$SRC_FOLDER/dde-dock-plugins/disk-mount/src.pri)

TR_EXCLUDE += $$SRC_FOLDER/dde-file-manager-lib/configure/*

INSTALLS += target gschema


QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

#include(../../../3rdparty/googletest/gtest_dependency.pri)
include(../../../3rdparty/cpp-stub/stub.pri)
include(test.pri)

unix {
    LIBS += -lgtest -lgmock
}
