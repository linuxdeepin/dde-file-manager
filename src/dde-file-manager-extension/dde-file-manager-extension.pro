#-------------------------------------------------
#
# Project created by QtCreator 2021-10-11T21:04:52
#
#-------------------------------------------------
include(../common/common.pri)

QT       -= core gui
CONFIG += c++14 create_pc create_prl no_install_prl

TARGET = dfm-extension
TEMPLATE = lib

DEFINES += DDEFILEMANAGEREXTENSION_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QMAKE_TARGET=\\\"$$TARGET\\\" QMAKE_VERSION=\\\"$$VERSION\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    menu/dfmextmenuproxy.cpp \
    menu/dfmextmenuplugin.cpp \
    menu/dfmextmenu.cpp \
    menu/dfmextaction.cpp \
    menu/private/dfmextactionprivate.cpp \
    menu/private/dfmextmenuprivate.cpp \
    menu/private/dfmextmenuproxyprivate.cpp \
    emblemicon/dfmextemblemiconplugin.cpp \
    emblemicon/dfmextemblem.cpp \
    emblemicon/private/dfmextemblemprivate.cpp \
    emblemicon/dfmextemblemiconlayout.cpp

HEADERS += \
    dfm-extension-global.h \
    dfm-extension.h \
    menu/dfmextmenuproxy.h \
    menu/dfmextmenuplugin.h \
    menu/dfmextmenu.h \
    menu/dfmextaction.h \
    menu/private/dfmextactionprivate.h \
    menu/private/dfmextmenuprivate.h \
    menu/private/dfmextmenuproxyprivate.h \
    emblemicon/dfmextemblemiconplugin.h \
    emblemicon/dfmextemblem.h \
    emblemicon/private/dfmextemblemprivate.h \
    emblemicon/dfmextemblemiconlayout.h \
    emblemicon/private/dfmextemblemiconlayoutprivate.h

DISTFILES += \
    .readme

isEmpty(PREFIX): PREFIX = /usr

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$[QT_INSTALL_LIBS]
} else {
    target.path = $$LIB_INSTALL_DIR
}

isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/dfm-extension
} else {
    includes.path = $$INCLUDE_INSTALL_DIR/dfm-extension
}

includes.files += $$PWD/*.h

menu.files += $$PWD/menu/*.h
menu.path = $$includes.path/menu

emblemicon.files += $$PWD/emblemicon/*.h
emblemicon.path = $$includes.path/emblemicon

plugindir.files = $$PWD/.readme
plugindir.path = $${target.path}/dde-file-manager/plugins/extensions

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_NAME = dfm-extension
QMAKE_PKGCONFIG_DESCRIPTION = Extension library of dde-file-manager which is provided for third parties
QMAKE_PKGCONFIG_INCDIR = $$includes.path

INSTALLS += target includes menu emblemicon plugindir
