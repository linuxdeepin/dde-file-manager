SOURCES += \
    $$PWD/dfmdiskdevice.cpp \
    $$PWD/dfmdiskmanager.cpp \
    $$PWD/udisks2_dbus_common.cpp \
    $$PWD/dfmblockdevice.cpp \
    $$PWD/dfmblockpartition.cpp

udisk2.files = $$PWD/org.freedesktop.UDisks2.xml
udisk2.header_flags = -i $$PWD/udisks2_dbus_common.h -N

DBUS_INTERFACES += udisk2 $$PWD/org.freedesktop.UDisks2.ObjectManager.xml

HEADERS += \
    $$PWD/dfmdiskdevice.h \
    $$PWD/udisks2_dbus_common.h \
    $$PWD/dfmdiskmanager.h \
    $$PWD/dfmblockdevice.h \
    $$PWD/dfmblockpartition.h

include($$PWD/private/private.pri)

includes.files += $$PWD/*.h
includes_private.files += $$PWD/private/*.h

INCLUDEPATH += $$PWD

OTHER_FILES += $$PWD/*.xml
