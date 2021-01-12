#-------------------------------------------------
#
# Project created by QtCreator 2017-03-31T16:00:06
#
#-------------------------------------------------

QT       += widgets   quick

TARGET = test-dde-image-preview-plugin
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += dtkwidget dtkgui

include(../../../common/common.pri)

LIBS += -L$$OUT_PWD/../../../dde-file-manager-lib -ldde-file-manager

DISTFILES += dde-image-preview-plugin.json

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(dde-image-preview-plugin.pri)
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

