HEADERS += \
    $$PWD/dfmmediainfo.h

SOURCES += \
    $$PWD/dfmmediainfo.cpp

# make the 'stdlib.h' not exist errors go away when adding the setting QMAKE_CFLAGS_ISYSTEM=-I
QMAKE_CFLAGS_ISYSTEM=-I
PKGCONFIG += libmediainfo
