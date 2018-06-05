INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/dabstractfiledevice.h \
    $$PWD/dlocalfiledevice.h \
    $$PWD/dfileiodeviceproxy.h

SOURCES += \
    $$PWD/dabstractfiledevice.cpp \
    $$PWD/dlocalfiledevice.cpp \
    $$PWD/dfileiodeviceproxy.cpp

include(private/private.pri)
