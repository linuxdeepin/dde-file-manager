INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/RollingFileAppender.h \
    $$PWD/Logger.h \
    $$PWD/FileAppender.h \
    $$PWD/CuteLogger_global.h \
    $$PWD/ConsoleAppender.h \
    $$PWD/AbstractStringAppender.h \
    $$PWD/AbstractAppender.h \
    $$PWD/logmanager.h

SOURCES += \
    $$PWD/RollingFileAppender.cpp \
    $$PWD/Logger.cpp \
    $$PWD/FileAppender.cpp \
    $$PWD/ConsoleAppender.cpp \
    $$PWD/AbstractStringAppender.cpp \
    $$PWD/AbstractAppender.cpp \
    $$PWD/logmanager.cpp

win32 {
    SOURCES += $$PWD/OutputDebugAppender.cpp
    HEADERS += $$PWD/OutputDebugAppender.h
}
