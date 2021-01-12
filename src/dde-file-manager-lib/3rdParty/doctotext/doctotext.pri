
CONFIG += c++11 link_pkgconfig
PKGCONFIG += libxml-2.0 htmlcxx
LIBS += -lz \
    -lmimetic
include($$PWD/../unzip101e/unzip.pri)
include($$PWD/../charsetdetect/charsetdetect.pri)
include($$PWD/../wv2/wv2.pri)

HEADERS += \
    $$PWD/*.h

SOURCES += \
    $$PWD/*.cpp

INCLUDEPATH += $$PWD/.. \
    $$PWD/../charsetdetect \
    $$PWD/../wv2 \
    $$PWD
