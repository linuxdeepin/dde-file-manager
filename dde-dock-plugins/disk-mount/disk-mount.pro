PREFIX = /usr
QT              += widgets
TEMPLATE         = lib
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkbase-0-2 dtkwidget-0-2 dde-file-manager gio-unix-2.0

INCLUDEPATH += /usr/include/dde-dock

TARGET          = $$qtLibraryTarget(disk-mount)
DESTDIR          = $$_PRO_FILE_PWD_/../
DISTFILES       += disk-mount.json

DEFINES += QT_MESSAGELOGCONTEXT

unix: LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

HEADERS += \
    diskmountplugin.h \
    diskcontrolwidget.h \
    diskpluginitem.h \
    diskcontrolitem.h

SOURCES += \
    diskmountplugin.cpp \
    diskcontrolwidget.cpp \
    diskpluginitem.cpp \
    diskcontrolitem.cpp

target.path = $${PREFIX}/lib/dde-dock/plugins/
INSTALLS += target

RESOURCES += \
    resources.qrc
