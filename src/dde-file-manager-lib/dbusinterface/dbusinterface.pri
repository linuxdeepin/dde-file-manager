HEADERS += \
    $$PWD/diskmount_interface.h \
    $$PWD/dbustype.h \
    $$PWD/usershare_interface.h \
    $$PWD/startmanager_interface.h \
    $$PWD/introspectable_interface.h \
    $$PWD/vault_interface.h \
    $$PWD/revocationmgr_interface.h \
    $$PWD/vaultbruteforceprevention_interface.h

SOURCES += \
    $$PWD/diskmount_interface.cpp \
    $$PWD/dbustype.cpp \
    $$PWD/usershare_interface.cpp \
    $$PWD/startmanager_interface.cpp \
    $$PWD/introspectable_interface.cpp \
    $$PWD/vault_interface.cpp \
    $$PWD/revocationmgr_interface.cpp \
    $$PWD/vaultbruteforceprevention_interface.cpp

!CONFIG(DISABLE_ANYTHING) {
    dbus_anything.files = /usr/share/dbus-1/interfaces/com.deepin.anything.xml
    DBUS_INTERFACES += dbus_anything
}
