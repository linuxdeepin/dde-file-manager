TEMPLATE = app
CONFIG -= qt

include(../common.pri)

#安全加固
QMAKE_CXXFLAGS += -fstack-protector-all
QMAKE_LFLAGS += -z now -pie -fPIE

SOURCES += \
    main.cpp \
    funcwrapper.cpp

!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    LIBS += -lffmpegthumbnailer
}

HEADERS += \
    funcwrapper.h
