CONFIG += link_pkgconfig
PKGCONFIG += poppler-cpp

HEADERS += \
    $$PWD/thumbnailmanager.h \
    $$PWD/thumbnailgenerator.h

SOURCES += \
    $$PWD/thumbnailmanager.cpp \
    $$PWD/thumbnailgenerator.cpp
