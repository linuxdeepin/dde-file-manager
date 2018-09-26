SOURCES += \
    $$PWD/dfmvfsmanager.cpp \
    $$PWD/dfmvfsdevice.cpp \
    $$PWD/dfmvfsabstracteventhandler.cpp

HEADERS += \
    $$PWD/dfmvfsmanager.h \
    $$PWD/dfmvfsdevice.h \
    $$PWD/dfmvfsabstracteventhandler.h

include($$PWD/private/private.pri)

INCLUDEPATH += $$PWD
