#-------------------------------------------------
#
# Project created by QtCreator 2020-07-15T11:45:51
#
#-------------------------------------------------

QT       += core gui testlib dbus-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#include(../../3rdparty/googletest/gtest_dependency.pri)
include(src/test.pri)
include(../../3rdparty/cpp-stub/stub.pri)
include(../../src/dde-desktop/dde-desktop.pri)

unix {
    LIBS += -lgtest -lgmock
}

QMAKE_CXXFLAGS += -fno-inline -fno-access-control

TARGET = test-dde-desktop
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += UT_DDE_DESKTOP_UNIONTECH

isEqual(ARCH, aarch64){
    DEFINES += __arm__
}

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        $$PWD/main.cpp

HEADERS += \

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0
