#-------------------------------------------------
#
# Project created by QtCreator 2017-03-31T16:00:06
#
#-------------------------------------------------

QT       += widgets

TARGET = dde-image-preview-plugin
TEMPLATE = lib

CONFIG += plugin c++11 link_pkgconfig
PKGCONFIG += dtkwidget

include(../../../common/common.pri)

SOURCES += \
    imageview.cpp \
    main.cpp \
    imagepreview.cpp

HEADERS += \
    imageview.h \
    imagepreview.h
DISTFILES += dde-image-preview-plugin.json

PLUGIN_INSTALL_DIR = $$PLUGINDIR/previews

DESTDIR = $$top_srcdir/plugins/previews

unix {
    target.path = $$PLUGIN_INSTALL_DIR
    INSTALLS += target
}
