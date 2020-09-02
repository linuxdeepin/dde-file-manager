#-------------------------------------------------
#
# Project created by QtCreator 2017-04-20T15:40:08
#
#-------------------------------------------------

QT       += core gui widgets multimedia quick

TARGET = test-dde-music-preview-plugin
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += taglib

include(../../../common/common.pri)

LIBS += -L$$OUT_PWD/../../../../build-filemanager-unknown-Debug/dde-file-manager-lib -ldde-file-manager -licui18n

DISTFILES += dde-music-preview-plugin.json


QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0 -DGTEST
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(dde-music-preview-plugin.pri)
include(../../../third-party/googletest/gtest_dependency.pri)
include(tests/test.pri)

