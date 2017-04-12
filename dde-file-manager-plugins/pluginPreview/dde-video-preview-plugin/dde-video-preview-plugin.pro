#-------------------------------------------------
#
# Project created by QtCreator 2017-04-24T16:11:22
#
#-------------------------------------------------

QT       += core gui widgets multimedia multimediawidgets x11extras

TARGET = dde-video-preview-plugin
TEMPLATE = lib

PKGCONFIG += mpv x11 x11-xcb
CONFIG += plugin link_pkgconfig c++11

include(../../plugininterfaces/plugininterfaces.pri)
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

SOURCES += videopreviewplugin.cpp \
    dvideowidget.cpp

HEADERS += videopreviewplugin.h \
    dvideowidget.h
DISTFILES += \
    dde-video-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/preview

DESTDIR = ../../preview

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
