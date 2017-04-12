#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T09:08:50
#
#-------------------------------------------------

QT       += core gui widgets concurrent

TARGET = dde-pdf-preview-plugin
TEMPLATE = lib

PKGCONFIG += poppler-cpp

CONFIG += plugin link_pkgconfig

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

SOURCES += pdfpreviewplugin.cpp \
    pdfwidget.cpp

HEADERS += pdfpreviewplugin.h \
    pdfwidget.h
DISTFILES += dde-pdf-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/preview

DESTDIR = ../../preview

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}

RESOURCES += \
    theme.qrc
