#-------------------------------------------------
#
# Project created by QtCreator 2017-04-20T15:40:08
#
#-------------------------------------------------

QT       += core gui widgets multimedia

TARGET = dde-music-preview-plugin
TEMPLATE = lib

PKGCONFIG += taglib

CONFIG += c++11 plugin link_pkgconfig

include(../../../common/common.pri)
include(dde-music-preview-plugin.pri)

DISTFILES += dde-music-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/previews

DESTDIR = $$top_srcdir/plugins/previews

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}

