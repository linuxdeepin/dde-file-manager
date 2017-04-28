#-------------------------------------------------
#
# Project created by QtCreator 2017-03-31T16:00:06
#
#-------------------------------------------------

QT       += widgets

TARGET = dde-image-preview-plugin
TEMPLATE = lib

CONFIG += plugin c++11

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


RESOURCES +=

SOURCES += \
    imagepreviewplugin.cpp \
    imageview.cpp \
    messagetoobar.cpp

HEADERS += \
    imagepreviewplugin.h \
    imageview.h \
    messagetoobar.h
DISTFILES += dde-image-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/preview

DESTDIR = ../../preview

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
