#-------------------------------------------------
#
# Project created by QtCreator 2017-04-17T11:02:31
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = dde-text-preview-plugin
TEMPLATE = lib
CONFIG += plugin

include(../../../common/common.pri)

SOURCES += \
    main.cpp \
    textpreview.cpp

HEADERS += \
    textpreview.h
DISTFILES += \
    dde-text-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/previews

DESTDIR = $$top_srcdir/plugins/previews

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
