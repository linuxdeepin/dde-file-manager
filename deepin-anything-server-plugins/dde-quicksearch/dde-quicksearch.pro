#-------------------------------------------------
#
# Project created by QtCreator 2018-06-25T09:04:00
#
#-------------------------------------------------

QT       += core gui

TARGET = dde-quicksearch


TEMPLATE = lib


QT += core \
      dbus \
      concurrent

CONFIG += c++11 \
          console \
          link_pkgconfig \
          plugin

CONFIG -= app_bundle \
          create_pc \
          create_prl \
          no_install_prl

isEmpty(DEPENDENCE_DIR){
    DEPENDENCE_DIR = $$PWD/../../dde-file-manager-lib
}

PKGCONFIG += deepin-anything-server-lib


INCLUDEPATH += $$PWD/../../dde-file-manager-lib \
               $$PWD/../../dde-file-manager-lib/interfaces \
               $$PWD/../../dde-file-manager-lib/shutil

unix{
      PKG_CONFIG = pkg-config
      target.path = $$system($$PKG_CONFIG --variable libdir deepin-anything-server-lib)/deepin-anything-server-lib/plugins/handlers
      INSTALLS += target
}


#DESTDIR = $$[QT_INSTALL_PLUGINS]/generic


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$DEPENDENCE_DIR/quick_search/dquicksearch.cpp \
    main.cpp \
    quicksearchhandle.cpp

HEADERS += \
    $$DEPENDENCE_DIR/quick_search/dquicksearch.h \
    quicksearchhandle.h

DISTFILES += dde-quicksearch.json
