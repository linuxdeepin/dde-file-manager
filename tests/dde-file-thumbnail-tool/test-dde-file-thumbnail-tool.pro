
TARGET = test-video
TEMPLATE = app

CONFIG += console


!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    LIBS += -lffmpegthumbnailer
}

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(../../src/common/common.pri)
#include(../../3rdparty/googletest/gtest_dependency.pri)
include(tests/test.pri)

unix {
    LIBS += -lgtest -lgmock
}

HEADERS += \
    $$PWD/../../src/dde-file-thumbnail-tool/video/funcwrapper.h

SOURCES += \
    $$PWD/../../src/dde-file-thumbnail-tool/video/funcwrapper.cpp

!CONFIG(DISABLE_TSAN_TOOL) {
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
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
