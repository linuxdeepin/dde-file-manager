PREFIX = /usr
QT              += core widgets concurrent dbus testlib
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkwidget gio-unix-2.0 gio-qt udisks2-qt5

INCLUDEPATH += /usr/include/dde-dock
INCLUDEPATH += $$PWD/../../dde-file-manager-lib/interfaces \
               $$PWD/../../dde-file-manager-lib


TARGET = test-dde-disk-mount-plugin
TEMPLATE = app

#DESTDIR          = $$_PRO_FILE_PWD_/../
DISTFILES       += disk-mount.json

DEFINES += QT_MESSAGELOGCONTEXT DFM_NO_FILE_WATCHER

include(src.pri)

TR_EXCLUDE += $$PWD/../../dde-file-manager-lib/configure/*

INSTALLS += target gschema translations

RESOURCES += \
    resources.qrc \
    resources/theme-icons.qrc \
    $$PWD/../../dde-file-manager-lib/configure.qrc

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(../../../3rdparty/googletest/gtest_dependency.pri)
include(../../third-party/cpp-stub/stub.pri)
include(tests/test.pri)
