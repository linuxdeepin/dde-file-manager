

unix: CONFIG(release, release|debug): LIBS += -lanything -L/usr/include/deepin-anything
else: unix: LIBS += -lanything -L/usr/include/deepin-anything


HEADERS += \
    $$PWD/dquicksearch.h

SOURCES += \
    $$PWD/dquicksearch.cpp
