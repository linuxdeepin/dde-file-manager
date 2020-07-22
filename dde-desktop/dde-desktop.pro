#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

include($$PWD/dde-desktop-build.pri)
include($$PWD/util/util.pri)
include($$PWD/dbus/dbus.pri)

include($$PWD/../dde-wallpaper-chooser/dde-wallpaper-chooser.pri)
include($$PWD/../dde-zone/dde-zone.pri)

QT       += core gui widgets svg dbus x11extras network concurrent multimediawidgets multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

greaterThan(QT_MINOR_VERSION, 7): QT += gui-private
else: QT += platformsupport-private

TEMPLATE    = app
TARGET      = dde-desktop
DESTDIR     = $$BUILD_DIST
CONFIG      += c++11 link_pkgconfig
PKGCONFIG   += xcb xcb-ewmh xcb-shape dframeworkdbus gio-qt xcursor

INCLUDEPATH += $$PWD/../dde-file-manager-lib\
               $$PWD/../utils \
              $$PWD/../dde-file-manager-lib/interfaces

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/release -ldde-file-manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/debug -ldde-file-manager
else:unix: LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

CONFIG(debug, debug|release) {
    DEPENDPATH += $$PWD/../dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../dde-file-manager-lib
}

SOURCES += \
    main.cpp \
    config/config.cpp \
    desktop.cpp \
    view/canvasviewhelper.cpp \
#    view/canvasview.cpp \
    model/dfileselectionmodel.cpp \
    view/canvasgridview.cpp \
    presenter/apppresenter.cpp \
    presenter/gridmanager.cpp \
    dbus/dbusdisplay.cpp \
    presenter/display.cpp \
    presenter/dfmsocketinterface.cpp \
    view/watermaskframe.cpp \
    dbus/dbusdock.cpp \
    config/profile.cpp \
    view/desktopitemdelegate.cpp \
    presenter/gridcore.cpp \
    config/dfmdesktopsettings.cpp \
    view/backgroundhelper.cpp \
    dbus/dbusmonitor.cpp



HEADERS += \
    config/config.h \
    desktop.h \
    view/canvasviewhelper.h \
    model/dfileselectionmodel.h \
    view/private/canvasviewprivate.h \
    global/coorinate.h \
    global/singleton.h \
    view/canvasgridview.h \
    presenter/apppresenter.h \
    presenter/gridmanager.h \
    dbus/dbusdisplay.h \
    presenter/display.h \
    presenter/dfmsocketinterface.h \
    view/watermaskframe.h \
    dbus/dbusdock.h \
    config/profile.h \
    view/desktopitemdelegate.h \
    presenter/gridcore.h \
    deventfilter.h \
    config/dfmdesktopsettings.h \
    view/backgroundhelper.h \
    dbus/dbusmonitor.h \
    accessible/accessiblelist.h \
    accessible/frameaccessibledefine.h

RESOURCES +=

# Automating generation .qm files from .ts files

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
    !system($$PWD/translate_ts2desktop.sh): error("Failed to generate translation")
}

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
                $$PWD/translations/$${TARGET}_zh_CN.ts


include($$PWD/development.pri)

