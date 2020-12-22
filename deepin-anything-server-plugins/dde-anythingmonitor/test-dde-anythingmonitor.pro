#-------------------------------------------------
#
# Project created by QtCreator 2018-06-22T11:31:57
#
#-------------------------------------------------


TARGET = test-dde-anythingmonitor

TEMPLATE = app

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

include(src.pri)
#include($$PWD/../../dde-file-manager-lib/fulltextsearch/fulltextsearch.pri)
DISTFILES += dde-anythingmonitor.json


QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(../../common/common.pri)
include(../../third-party/googletest/gtest_dependency.pri)
include(tests/test.pri)

#内存检测标签
TSAN_TOOL_ENABLE = true
equals(TSAN_TOOL_ENABLE, true ){
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
    message("deepin-screen-recorder enabled TSAN function with set: " $$TSAN_TOOL_ENABLE)
    contains(DEFINES, TSAN_THREAD){
       QMAKE_CXXFLAGS+="-fsanitize=thread"
       QMAKE_CFLAGS+="-fsanitize=thread"
       QMAKE_LFLAGS+="-fsanitize=thread"
    } else {
       QMAKE_CXXFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_CFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_LFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
    }
}
