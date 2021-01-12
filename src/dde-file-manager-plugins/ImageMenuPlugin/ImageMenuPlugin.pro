#-------------------------------------------------
#
# Project created by QtCreator 2016-11-29T11:20:23
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageMenuPlugin
TEMPLATE = lib
CONFIG += plugin c++11

include(../../common/common.pri)
include(../plugininterfaces/plugininterfaces.pri)

SOURCES += imagemenuplugin.cpp

HEADERS += imagemenuplugin.h
DISTFILES += ImageMenuPlugin.json

DESTDIR = ../menu

unix {
    target.path = $$PLUGINDIR/menu
    INSTALLS += target
}

RESOURCES += \
    pluginicons.qrc
