include($$PWD/dde-desktop-build.pri)
include($$PWD/util/util.pri)
include($$PWD/dbus/dbus.pri)

include($$PWD/../dde-wallpaper-chooser/dde-wallpaper-chooser.pri)
#无热区设置
#include($$PWD/../dde-zone/dde-zone.pri)
DEFINES += DISABLE_ZONE

QT       += core gui widgets dbus x11extras network concurrent multimedia #multimediawidgets svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

greaterThan(QT_MINOR_VERSION, 7): QT += gui-private
else: QT += platformsupport-private

CONFIG      += c++11 link_pkgconfig
PKGCONFIG   += xcb xcb-ewmh xcb-shape dframeworkdbus gio-qt

INCLUDEPATH += $$PWD/../dde-file-manager-lib\
                $$PWD/../dde-file-manager-lib/interfaces \
                $$PWD/../dde-file-manager-lib/interfaces/plugins \
                $$PWD/../dde-file-manager-lib/io\
                $$PWD/../utils\
                $$PWD/view \
                $$PWD

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/release -ldde-file-manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/debug -ldde-file-manager
else:unix: LIBS += -L$$OUT_PWD/../../src/dde-file-manager-lib -ldde-file-manager

CONFIG(debug, debug|release) {
    DEPENDPATH += $$PWD/../dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../../src/dde-file-manager-lib
}

SOURCES += \
    $$PWD/config/config.cpp \
    $$PWD/desktop.cpp \
    $$PWD/view/canvasviewhelper.cpp \
    $$PWD/model/dfileselectionmodel.cpp \
    $$PWD/view/canvasgridview.cpp \
    $$PWD/presenter/apppresenter.cpp \
    $$PWD/presenter/gridmanager.cpp \
    $$PWD/dbus/dbusdisplay.cpp \
    $$PWD/presenter/dfmsocketinterface.cpp \
    $$PWD/view/watermaskframe.cpp \
    $$PWD/dbus/dbusdock.cpp \
    $$PWD/view/desktopitemdelegate.cpp \
    $$PWD/presenter/gridcore.cpp \
    $$PWD/dbus/dbusmonitor.cpp \
    $$PWD/screen/abstractscreen.cpp \
    $$PWD/screen/abstractscreenmanager.cpp \
    $$PWD/screen/screenmanager.cpp \
    $$PWD/screen/screenobject.cpp \
    $$PWD/view/backgroundmanager.cpp \
    $$PWD/screen/screenhelper.cpp \
    $$PWD/view/backgroundwidget.cpp \
    $$PWD/screen/screenmanagerwayland.cpp \
    $$PWD/screen/screenobjectwayland.cpp \
    $$PWD/dbus/licenceInterface.cpp \
    $$PWD/view/canvasviewmanager.cpp \
    $$PWD/presenter/deepinlicensehelper.cpp



HEADERS += \
    $$PWD/config/config.h \
    $$PWD/desktop.h \
    $$PWD/view/canvasviewhelper.h \
    $$PWD/model/dfileselectionmodel.h \
    $$PWD/view/private/canvasviewprivate.h \
    $$PWD/global/coorinate.h \
    $$PWD/global/singleton.h \
    $$PWD/view/canvasgridview.h \
    $$PWD/presenter/apppresenter.h \
    $$PWD/presenter/gridmanager.h \
    $$PWD/dbus/dbusdisplay.h \
    $$PWD/presenter/dfmsocketinterface.h \
    $$PWD/view/watermaskframe.h \
    $$PWD/dbus/dbusdock.h \
    $$PWD/view/desktopitemdelegate.h \
    $$PWD/presenter/gridcore.h \
    $$PWD/deventfilter.h \
    $$PWD/dbus/dbusmonitor.h \
    $$PWD/dbus/licenceInterface.h \
    $$PWD/screen/abstractscreen.h \
    $$PWD/screen/abstractscreenmanager.h \
    $$PWD/screen/screenmanager.h \
    $$PWD/screen/screenobject.h \
    $$PWD/view/backgroundmanager.h \
    $$PWD/screen/screenhelper.h \
    $$PWD/view/backgroundwidget.h \
    $$PWD/screen/screenmanagerwayland.h \
    $$PWD/screen/screenobjectwayland.h \
    $$PWD/view/canvasviewmanager.h \
    $$PWD/screen/abstractscreenmanager_p.h \
    $$PWD/accessible/frameaccessibledefine.h \
    $$PWD/accessible/accessiblelist.h \
    $$PWD/presenter/deepinlicensehelper.h
