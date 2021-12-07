TEMPLATE = app
CONFIG -= qt

include(../common.pri)

#安全加固
QMAKE_CXXFLAGS += -fstack-protector-all
QMAKE_LFLAGS += -z now -pie -fPIE
isEqual(ARCH, mips64) | isEqual(ARCH, mips32){
    QMAKE_LFLAGS += -z noexecstack -z relro
}

SOURCES += \
    main.cpp \
    funcwrapper.cpp

!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    LIBS += -lffmpegthumbnailer
}

HEADERS += \
    funcwrapper.h
