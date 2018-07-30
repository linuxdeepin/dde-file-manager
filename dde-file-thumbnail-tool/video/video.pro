TEMPLATE = app
CONFIG -= qt

include(../common.pri)

SOURCES += \
    main.cpp

!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    LIBS += -lffmpegthumbnailer
}
