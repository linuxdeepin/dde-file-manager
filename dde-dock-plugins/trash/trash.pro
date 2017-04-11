isEmpty(PREFIX)
{
    PREFIX = /usr
}


QT              += widgets svg dbus
TEMPLATE         = lib
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkbase dtkwidget

INCLUDEPATH += /usr/include/dde-dock
INCLUDEPATH += $$PWD/../../dde-file-manager-lib/interfaces

unix: LIBS += -L$$OUT_PWD/../../dde-file-manager-lib -ldde-file-manager

TARGET          = $$qtLibraryTarget(dde-trash-plugin)
DESTDIR          = $$_PRO_FILE_PWD_/../
DISTFILES       += trash.json

DEFINES += QT_MESSAGELOGCONTEXT

HEADERS += \
    trashplugin.h \
    trashwidget.h \
    popupcontrolwidget.h

SOURCES += \
    trashplugin.cpp \
    trashwidget.cpp \
    popupcontrolwidget.cpp

target.path = $${PREFIX}/lib/dde-dock/plugins/
INSTALLS += target

RESOURCES += \
