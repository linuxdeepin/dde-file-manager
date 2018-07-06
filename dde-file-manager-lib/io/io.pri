INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/dlocalfiledevice.h \
    $$PWD/dfileiodeviceproxy.h \
    $$PWD/dfilecopymovejob.h \
    $$PWD/dfilehandler.h \
    $$PWD/dfiledevice.h \
    $$PWD/dlocalfilehandler.h \
    $$PWD/dfilestatisticsjob.h

SOURCES += \
    $$PWD/dlocalfiledevice.cpp \
    $$PWD/dfileiodeviceproxy.cpp \
    $$PWD/dfilecopymovejob.cpp \
    $$PWD/dfilehandler.cpp \
    $$PWD/dfiledevice.cpp \
    $$PWD/dlocalfilehandler.cpp \
    $$PWD/dfilestatisticsjob.cpp

include(private/private.pri)
