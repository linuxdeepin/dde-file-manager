QT += gui-private core-private x11extras
CONFIG += link_pkgconfig

PKGCONFIG += xcb

HEADERS += \
    $$PWD/xdndworkaround.h

SOURCES += \
    $$PWD/xdndworkaround.cpp

INCLUDEPATH += $$PWD
