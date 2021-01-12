#-------------------------------------------------
#
# Project created by QtCreator 2017-04-20T15:40:08
#
#-------------------------------------------------

QT       += core gui widgets multimedia quick

TARGET = test-dde-music-preview-plugin
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += taglib

include(../../../common/common.pri)

LIBS += -L$$OUT_PWD/../../../dde-file-manager-lib -ldde-file-manager -licui18n

DISTFILES += dde-music-preview-plugin.json

#DESTDIR += $$PWD/
#OBJECTS_DIR = $$PWD/debug/obj
#MOC_DIR = $$PWD/debug/moc

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0 -DGTEST
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(dde-music-preview-plugin.pri)
include(../../../../3rdparty/googletest/gtest_dependency.pri)
include(../../../../3rdparty/cpp-stub/stub.pri)
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
