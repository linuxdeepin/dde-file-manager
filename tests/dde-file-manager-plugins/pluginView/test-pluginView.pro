#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T09:08:50
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = test-pluginView
TEMPLATE = app

CONFIG += c++11 plugin

include(../../../src/common/common.pri)
include(../../../src/dde-file-manager-plugins/plugininterfaces/plugininterfaces.pri)

DISTFILES += pluginView.json

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0 -DGTEST
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0 -DGTEST

SOURCES += \
    $$PWD/../../../src/dde-file-manager-plugins/pluginView/viewplugin.cpp

HEADERS += \
    $$PWD/../../../src/dde-file-manager-plugins/pluginView/viewplugin.h

RESOURCES += \
    $$PWD/../../../src/dde-file-manager-plugins/pluginView/pluginview.qrc

#include(../../../3rdparty/googletest/gtest_dependency.pri)
include(../../../3rdparty/cpp-stub/stub.pri)
include(tests/test.pri)

unix {
    LIBS += -lgtest -lgmock
}

!CONFIG(DISABLE_TSAN_TOOL) {
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
    contains(DEFINES, TSAN_THREAD){
       QMAKE_CXXFLAGS+="-fsanitize=thread"
       QMAKE_CFLAGS+="-fsanitize=thread"
       QMAKE_LFLAGS+="-fsanitize=thread"
    } else {
       QMAKE_CXXFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_CFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_LFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
    }
}
