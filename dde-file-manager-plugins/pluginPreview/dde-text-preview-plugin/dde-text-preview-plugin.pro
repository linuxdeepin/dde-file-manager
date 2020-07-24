#-------------------------------------------------
#
# Project created by QtCreator 2017-04-17T11:02:31
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = dde-text-preview-plugin
TEMPLATE = lib
CONFIG += c++11 plugin

include(../../../common/common.pri)
include(dde-text-preview-plugin.pri)

DISTFILES += \
    dde-text-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/previews

DESTDIR = $$top_srcdir/plugins/previews

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
