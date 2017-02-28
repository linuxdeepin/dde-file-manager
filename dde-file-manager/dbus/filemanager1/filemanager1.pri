filemanager.files = $$PWD/org.freedesktop.FileManager1.xml
filemanager.header_flags += -l DBusFileManager1 -i $$PWD/dbusfilemanager1.h
filemanager.source_flags += -l DBusFileManager1

DBUS_ADAPTORS += filemanager

filemanager_xmls.path = /usr/share/dbus-1/interfaces
filemanager_xmls.files = $$filemanager.files

filemanager_service.path = /usr/share/dbus-1/services
filemanager_service.files = $$PWD/org.freedesktop.FileManager.service

INSTALLS += filemanager_service #filemanager_xmls

HEADERS += \
    $$PWD/dbusfilemanager1.h

SOURCES += \
    $$PWD/dbusfilemanager1.cpp

INCLUDEPATH += $$PWD
