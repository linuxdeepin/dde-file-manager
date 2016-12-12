QT += dbus

dialog.files = $$PWD/com.deepin.filemanager.filedialog.xml
dialog.header_flags += -l DBusFileDialogHandle -i $$PWD/dbusfiledialoghandle.h
dialog.source_flags += -l DBusFileDialogHandle

manager.files = $$PWD/com.deepin.filemanager.filedialogmanager.xml
manager.header_flags += -l DBusFileDialogManager -i $$PWD/dbusfiledialogmanager.h
manager.source_flags += -l DBusFileDialogManager

DBUS_ADAPTORS += dialog manager

xmls.path = /usr/share/dbus-1/interfaces
xmls.files = $$dialog.files $$manager.files

service.path = /usr/share/dbus-1/services
service.files = $$PWD/com.deepin.filemanager.filedialog.service

INSTALLS += service xmls

HEADERS += \
    $$PWD/dbusfiledialoghandle.h \
    $$PWD/dbusfiledialogmanager.h

SOURCES += \
    $$PWD/dbusfiledialoghandle.cpp \
    $$PWD/dbusfiledialogmanager.cpp
