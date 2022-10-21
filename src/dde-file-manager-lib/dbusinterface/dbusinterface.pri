3RDINTERFACE = $$PWD/../../../3rdpart/dbusservice
INCLUDEPATH += $$PWD \
               $$3RDINTERFACE


HEADERS += \
    $$PWD/dbustype.h \
    $$3RDINTERFACE/dbusinterface/disk_interface.h \
    $$3RDINTERFACE/dbusinterface/usershare_interface.h \
    $$3RDINTERFACE/dbusinterface/vault_interface.h \
    $$3RDINTERFACE/dbusinterface/revocationmgr_interface.h \
    $$3RDINTERFACE/dbusinterface/vaultbruteforceprevention_interface.h \
    $$3RDINTERFACE/dbusinterface/diskmount_interface.h \
    $$3RDINTERFACE/dbusinterface/introspectable_interface.h \
    $$3RDINTERFACE/dbusinterface/startmanager_interface.h \
    $$3RDINTERFACE/dbusinterface/tagmanagerdaemon_interface.h

SOURCES += \
    $$PWD/dbustype.cpp \
    $$3RDINTERFACE/dbusinterface/disk_interface.cpp \
    $$3RDINTERFACE/dbusinterface/usershare_interface.cpp \
    $$3RDINTERFACE/dbusinterface/vault_interface.cpp \
    $$3RDINTERFACE/dbusinterface/revocationmgr_interface.cpp \
    $$3RDINTERFACE/dbusinterface/vaultbruteforceprevention_interface.cpp \
    $$3RDINTERFACE/dbusinterface/diskmount_interface.cpp \
    $$3RDINTERFACE/dbusinterface/introspectable_interface.cpp \
    $$3RDINTERFACE/dbusinterface/startmanager_interface.cpp \
    $$3RDINTERFACE/dbusinterface/tagmanagerdaemon_interface.cpp

CONFIG(ENABLE_ANYTHING) {
    dbus_anything.files = /usr/share/dbus-1/interfaces/com.deepin.anything.xml
    DBUS_INTERFACES += dbus_anything
}
