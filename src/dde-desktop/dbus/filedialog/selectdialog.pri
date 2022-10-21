dialog.files = $$PWD/com.deepin.filemanager.filedialog.xml
dialog.header_flags += -l DBusFileDialogHandle -i $$PWD/dbusfiledialoghandle.h
dialog.source_flags += -l DBusFileDialogHandle

manager.files = $$PWD/com.deepin.filemanager.filedialogmanager.xml
manager.header_flags += -l DBusFileDialogManager -i $$PWD/dbusfiledialogmanager.h
manager.source_flags += -l DBusFileDialogManager

DBUS_ADAPTORS += dialog manager

HEADERS += \
    $$PWD/dbusfiledialoghandle.h \
    $$PWD/dbusfiledialogmanager.h

SOURCES += \
    $$PWD/dbusfiledialoghandle.cpp \
    $$PWD/dbusfiledialogmanager.cpp

INCLUDEPATH += $$PWD \
               $$PWD/../../../../3rdparty/dbusservice
DEFINES += PRO_FILE_PWD=\\\"$$_PRO_FILE_PWD_\\\"
