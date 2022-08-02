#-------------------------------------------------
#
# Project created by QtCreator 2018-06-22T11:31:57
#
#-------------------------------------------------


TARGET = dde-anythingmonitor

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


isEmpty(DDE_FILE_MANAGER_LIB_DIR){
    DDE_FILE_MANAGER_LIB_DIR = $$PWD/../../dde-file-manager-lib
}

isEmpty(DDE_FILE_MANAGER_DIR){
    DDE_FILE_MANAGER_DIR = $$PWD/../..
}


PKGCONFIG += deepin-anything-server-lib


INCLUDEPATH += $$PWD/../../dde-file-manager-lib \
               $$PWD/../../dde-file-manager-lib/interfaces \
               $$PWD/../../dde-file-manager-lib/shutil \
               $$PWD/../../utils

unix{
      PKG_CONFIG = pkg-config
      target.path = $$system($$PKG_CONFIG --variable libdir deepin-anything-server-lib)/deepin-anything-server-lib/plugins/handlers
      INSTALLS += target
}

#安全加固
QMAKE_CXXFLAGS += -fstack-protector-all
QMAKE_LFLAGS += -z now -fPIC
isEqual(ARCH, mips64) | isEqual(ARCH, mips32){
    QMAKE_LFLAGS += -z noexecstack -z relro
}

#DESTDIR = $$[QT_INSTALL_PLUGINS]/generic

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS DDE_ANYTHINGMONITOR DFM_NO_FILE_WATCHER

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$DDE_FILE_MANAGER_LIB_DIR/tag/tagmanager.cpp \
    $$DDE_FILE_MANAGER_LIB_DIR/shutil/danythingmonitorfilter.cpp \
    $$DDE_FILE_MANAGER_LIB_DIR/controllers/tagmanagerdaemoncontroller.cpp \
    $$DDE_FILE_MANAGER_LIB_DIR/controllers/interface/tagmanagerdaemon_interface.cpp \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/durl.cpp \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/dfmstandardpaths.cpp \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/dfmapplication.cpp \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/dfmsettings.cpp \
    taghandle.cpp \
    taghandleplugin.cpp


HEADERS += \
    $$DDE_FILE_MANAGER_DIR/utils/singleton.h \
    $$DDE_FILE_MANAGER_LIB_DIR/tag/tagmanager.h \
    $$DDE_FILE_MANAGER_LIB_DIR/shutil/danythingmonitorfilter.h \
    $$DDE_FILE_MANAGER_LIB_DIR/controllers/tagmanagerdaemoncontroller.h \
    $$DDE_FILE_MANAGER_LIB_DIR/controllers/interface/tagmanagerdaemon_interface.h \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/durl.h \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/dfmstandardpaths.h \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/dfmapplication.h \
    $$DDE_FILE_MANAGER_LIB_DIR/interfaces/dfmsettings.h \
    taghandle.h \
    taghandleplugin.h



DISTFILES += dde-anythingmonitor.json
