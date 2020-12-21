#-------------------------------------------------
#
# Project created by QtCreator 2017-03-31T16:00:06
#
#-------------------------------------------------

QT       += widgets   quick

TARGET = test-dde-image-preview-plugin
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += dtkwidget dtkgui

include(../../../common/common.pri)

LIBS += -L$$OUT_PWD/../../../dde-file-manager-lib -ldde-file-manager

DISTFILES += dde-image-preview-plugin.json

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(dde-image-preview-plugin.pri)
include(../../../third-party/googletest/gtest_dependency.pri)
include(../../../third-party/cpp-stub/stub.pri)
include(tests/test.pri)


