# Still need the widgets module since MenuInterface need it.
QT       += widgets

CONFIG   += link_pkgconfig
PKGCONFIG += dde-file-manager

TARGET = dde-file-manager-mounter-plugin
TEMPLATE = lib

DEFINES += DFMMOUNTERPLUGIN_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        dfmctxmenuplugin.cpp

HEADERS += \
        dfmctxmenuplugin.h

unix {
    target.path = $$[QT_INSTALL_LIBS]/dde-file-manager/plugins/menu/
    INSTALLS += target
}
