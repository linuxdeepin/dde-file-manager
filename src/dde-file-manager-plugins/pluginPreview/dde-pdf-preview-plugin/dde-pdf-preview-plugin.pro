#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T09:08:50
#
#-------------------------------------------------

QT       += core gui widgets concurrent

TARGET = dde-pdf-preview-plugin
TEMPLATE = lib

PKGCONFIG += poppler-cpp

CONFIG += c++11 plugin link_pkgconfig

include(../../../common/common.pri)
include(dde-pdf-preview-plugin.pri)

DISTFILES += dde-pdf-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/previews

DESTDIR = $$top_srcdir/plugins/previews

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
