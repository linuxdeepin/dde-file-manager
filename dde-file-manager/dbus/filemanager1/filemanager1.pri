filemanager.files = $$PWD/org.freedesktop.FileManager1.xml
filemanager.header_flags += -l DBusFileManager1 -i $$PWD/dbusfilemanager1.h
filemanager.source_flags += -l DBusFileManager1

DBUS_ADAPTORS += filemanager

xmls.path = /usr/share/dbus-1/interfaces
xmls.files = $$filemanager.files

service.path = /usr/share/dbus-1/services
service.files = $$PWD/org.freedesktop.FileManager.service

INSTALLS += service xmls

HEADERS += \
    $$PWD/dbusfilemanager1.h

SOURCES += \
    $$PWD/dbusfilemanager1.cpp

INCLUDEPATH += $$PWD
