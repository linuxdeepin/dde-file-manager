#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T09:08:50
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = test-pluginView
TEMPLATE = app

CONFIG += c++11 plugin

include(../../common/common.pri)
include(../plugininterfaces/plugininterfaces.pri)

RESOURCES += \
    pluginview.qrc

DISTFILES += pluginView.json

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0 -DGTEST
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0 -DGTEST

include(pluginView.pri)
include(../../third-party/googletest/gtest_dependency.pri)
include(../../third-party/cpp-stub/stub.pri)
include(tests/test.pri)

