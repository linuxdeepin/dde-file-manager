#-------------------------------------------------
#
# Project created by QtCreator 2016-12-21T13:15:17
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pluginView
TEMPLATE = lib
CONFIG += c++11 plugin

include(../../common/common.pri)
include(../plugininterfaces/plugininterfaces.pri)

#安全加固
QMAKE_CXXFLAGS += -fstack-protector-all
QMAKE_LFLAGS += -z now -fPIC

DESTDIR = ../view

SOURCES += \
    viewplugin.cpp

HEADERS += \
    viewplugin.h
DISTFILES += pluginView.json

unix {
    target.path = $$PLUGINDIR/view
    INSTALLS += target
}

RESOURCES += \
    pluginview.qrc
