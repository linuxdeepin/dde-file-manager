#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

include(../../src/common/common.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += network concurrent

TARGET = test-file-manager

TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += gio-unix-2.0
CONFIG(release, release|debug) {
    PKGCONFIG += dtkwidget
} else {
    PKGCONFIG += dtkwidget
}

DEFINES += QMAKE_TARGET=\\\"$$TARGET\\\" QMAKE_VERSION=\\\"$$VERSION\\\"

DEFINES += QT_MESSAGELOGCONTEXT

isEmpty(QMAKE_ORGANIZATION_NAME) {
    DEFINES += QMAKE_ORGANIZATION_NAME=\\\"deepin\\\"
}

isEmpty(PREFIX){
    PREFIX = /usr
}

CONFIG(release, debug|release) {
    !system($$PWD/translate_ts2desktop.sh): error("Failed to generate translation")
}

CONFIG(debug, debug|release) {
#    LIBS += -lprofiler
#    DEFINES += ENABLE_PPROF
}


INCLUDEPATH += $$PWD/../../src/dde-file-manager-lib $$PWD/.. \
               $$PWD/../../src/utils \
               $$PWD/../../src/dde-file-manager-lib/interfaces

BINDIR = $$PREFIX/bin
DEFINES += APPSHAREDIR=\\\"$$PREFIX/share/$$TARGET\\\"


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/dde-file-manager-lib/release -ldde-file-manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/dde-file-manager-lib/debug -ldde-file-manager
else:unix: LIBS += -L$$OUT_PWD/../../src/dde-file-manager-lib -ldde-file-manager

CONFIG(debug, debug|release) {
    DEPENDPATH += $$PWD/../../src/dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../../src/dde-file-manager-lib
}

HEADERS += \
    $$PWD/../../src/dde-file-manager/filemanagerapp.h \
    $$PWD/../../src/dde-file-manager/logutil.h \
    $$PWD/../../src/dde-file-manager/singleapplication.h \
    $$PWD/../../src/dde-file-manager/commandlinemanager.h

SOURCES += \
    $$PWD/../../src/dde-file-manager/filemanagerapp.cpp \
    $$PWD/../../src/dde-file-manager/logutil.cpp \
    $$PWD/../../src/dde-file-manager/singleapplication.cpp \
    $$PWD/../../src/dde-file-manager/commandlinemanager.cpp


QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

#include(../../3rdparty/googletest/gtest_dependency.pri)
include(../../3rdparty/cpp-stub/stub.pri)
include(test.pri)

unix {
    LIBS += -lgtest -lgmock
}
