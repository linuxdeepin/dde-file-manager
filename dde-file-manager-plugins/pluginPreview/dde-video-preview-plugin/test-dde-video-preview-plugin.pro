#-------------------------------------------------
#
# Project created by QtCreator 2017-04-24T16:11:22
#
#-------------------------------------------------

QT       += core gui widgets quick

TARGET = test-dde-video-preview-plugin
TEMPLATE = app

PKGCONFIG += libdmr dtkgui
CONFIG += console link_pkgconfig c++11

include(../../../common/common.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


LIBS += -L$$OUT_PWD/../../../dde-file-manager-lib -ldde-file-manager -licui18n

DISTFILES += \
    dde-video-preview-plugin.json

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(dde-video-preview-plugin.pri)
include(../../../third-party/googletest/gtest_dependency.pri)
include(../../../third-party/cpp-stub/stub.pri)
include(tests/test.pri)
