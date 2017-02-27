dialog.files = $$PWD/com.deepin.filemanager.filedialog.xml
dialog.header_flags += -l DBusFileDialogHandle -i $$PWD/dbusfiledialoghandle.h
dialog.source_flags += -l DBusFileDialogHandle

manager.files = $$PWD/com.deepin.filemanager.filedialogmanager.xml
manager.header_flags += -l DBusFileDialogManager -i $$PWD/dbusfiledialogmanager.h
manager.source_flags += -l DBusFileDialogManager

DBUS_ADAPTORS += dialog manager

filedialog_xmls.path = /usr/share/dbus-1/interfaces
filedialog_xmls.files = $$dialog.files $$manager.files

filedialog_service.path = /usr/share/dbus-1/services
filedialog_service.files = $$PWD/com.deepin.filemanager.filedialog.service

filedialog_blacklist.path = /usr/share/$${TARGET}/dbusfiledialog
filedialog_blacklist.files = $$PWD/dbus_filedialog_blacklist.conf

INSTALLS += filedialog_service filedialog_xmls filedialog_blacklist

HEADERS += \
    $$PWD/dbusfiledialoghandle.h \
    $$PWD/dbusfiledialogmanager.h

SOURCES += \
    $$PWD/dbusfiledialoghandle.cpp \
    $$PWD/dbusfiledialogmanager.cpp

INCLUDEPATH += $$PWD
DEFINES += PRO_FILE_PWD=\\\"$$_PRO_FILE_PWD_\\\"
