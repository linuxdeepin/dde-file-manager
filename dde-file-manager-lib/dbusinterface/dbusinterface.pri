HEADERS += \
    $$PWD/diskmount_interface.h \
    $$PWD/dbustype.h \
    $$PWD/usershare_interface.h \
    $$PWD/deviceinfomanager_interface.h \
    $$PWD/commandmanager_interface.h \
    $$PWD/startmanager_interface.h \
    $$PWD/introspectable_interface.h

SOURCES += \
    $$PWD/diskmount_interface.cpp \
    $$PWD/dbustype.cpp \
    $$PWD/usershare_interface.cpp \
    $$PWD/deviceinfomanager_interface.cpp \
    $$PWD/commandmanager_interface.cpp \
    $$PWD/startmanager_interface.cpp \
    $$PWD/introspectable_interface.cpp

!CONFIG(DISABLE_ANYTHING) {
    dbus_anything.files = /usr/share/dbus-1/interfaces/com.deepin.anything.xml
    DBUS_INTERFACES += dbus_anything
}
