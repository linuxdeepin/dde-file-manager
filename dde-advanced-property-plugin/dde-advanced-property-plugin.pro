#-------------------------------------------------
#
# Project created by QtCreator 2017-03-15T16:03:35
#
#-------------------------------------------------
include(../common/common.pri)
QT       += core gui widgets

TARGET = dde-advanced-property-plugin
TEMPLATE = lib
CONFIG += plugin c++11

PLUGIN_INSTALL_DIR = $$PLUGINDIR/menu
message($$PLUGIN_INSTALL_DIR)

SOURCES += \
    dadvancedinfowidget.cpp \
    dadvancedinfoplugin.cpp

DESTDIR = ../dde-file-manager-plugins/menu

HEADERS += \
    dadvancedinfowidget.h \
    dadvancedinfoplugin.h
DISTFILES += dde-advanced-property-plugin.json

INCLUDEPATH += \
    ../dde-file-manager-lib \
    ../dde-file-manager-lib/interfaces

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}

RESOURCES += \
    advanced-info.qrc
