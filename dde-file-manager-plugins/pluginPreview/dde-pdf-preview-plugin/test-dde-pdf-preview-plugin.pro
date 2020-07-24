#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T09:08:50
#
#-------------------------------------------------

QT       += core gui widgets concurrent

TARGET = test-dde-pdf-preview-plugin
TEMPLATE = app

PKGCONFIG += poppler-cpp

CONFIG += c++11 console link_pkgconfig

include(../../../common/common.pri)

LIBS += -L$$OUT_PWD/../../../../build-filemanager-unknown-Debug/dde-file-manager-lib -ldde-file-manager

DISTFILES += dde-pdf-preview-plugin.json

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(dde-pdf-preview-plugin.pri)
include(../../../third-party/googletest/gtest_dependency.pri)
include(tests/test.pri)

