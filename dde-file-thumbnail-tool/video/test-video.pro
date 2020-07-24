
TARGET = test-video
TEMPLATE = app

CONFIG += console


!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    LIBS += -lffmpegthumbnailer
}

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(../../third-party/googletest/gtest_dependency.pri)
include(tests/test.pri)
