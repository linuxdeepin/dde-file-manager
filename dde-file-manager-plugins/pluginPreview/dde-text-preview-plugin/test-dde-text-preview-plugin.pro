#-------------------------------------------------
#
# Project created by QtCreator 2017-04-17T11:02:31
#
#-------------------------------------------------

QT       += core gui widgets quick

TARGET = test-dde-text-preview-plugin
TEMPLATE = app
CONFIG += c++11 console

include(../../../common/common.pri)

LIBS += -L$$OUT_PWD/../../../dde-file-manager-lib -ldde-file-manager

DISTFILES += \
    dde-text-preview-plugin.json
#DESTDIR += $$PWD/
#OBJECTS_DIR = $$PWD/debug/obj
#MOC_DIR = $$PWD/debug/moc

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(dde-text-preview-plugin.pri)
include(../../../third-party/googletest/gtest_dependency.pri)
include(../../../third-party/cpp-stub/stub.pri)
include(tests/test.pri)
