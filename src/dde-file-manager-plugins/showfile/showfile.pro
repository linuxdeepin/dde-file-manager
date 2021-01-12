#-------------------------------------------------
#
# Project created by QtCreator 2016-12-01T16:21:23
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = showfile
TEMPLATE = lib
CONFIG += c++11 plugin

include(../../common/common.pri)
include(../plugininterfaces/plugininterfaces.pri)

DESTDIR = ../menu

SOURCES += showfileplugin.cpp

HEADERS += showfileplugin.h
DISTFILES += showfile.json
RESOURCES +=  images.qrc

unix {
    target.path = $$PLUGINDIR/menu
    INSTALLS += target
}
