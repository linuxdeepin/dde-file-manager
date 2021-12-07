#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

include($$PWD/dde-desktop-build.pri)
include($$PWD/util/util.pri)
include($$PWD/dbus/dbus.pri)

include($$PWD/../dde-wallpaper-chooser/dde-wallpaper-chooser.pri)
# 集成测试标签
include($$PWD/../../3rdparty/accessibility/accessibility-suite.pri)
#无热区设置
#include($$PWD/../dde-zone/dde-zone.pri)
DEFINES += DISABLE_ZONE

# 获取标签系统设置
AC_FUNC_ENABLE = true
#AC_FUNC_ENABLE = $$(ENABLE_AC_FUNC)
# 检查集成测试标签
equals( AC_FUNC_ENABLE, true ){
    DEFINES += ENABLE_ACCESSIBILITY
    message("dde-desktop enabled accessibility function with set: " $$AC_FUNC_ENABLE)
}

QT       += core gui widgets dbus x11extras network concurrent multimedia #multimediawidgets svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

greaterThan(QT_MINOR_VERSION, 7): QT += gui-private
else: QT += platformsupport-private

TEMPLATE    = app
TARGET      = dde-desktop
DESTDIR     = $$BUILD_DIST
CONFIG      += c++11 link_pkgconfig
PKGCONFIG   += xcb xcb-ewmh xcb-shape dframeworkdbus gio-qt

INCLUDEPATH += $$PWD/../dde-file-manager-lib\
               $$PWD/../utils \
              $$PWD/../dde-file-manager-lib/interfaces \
                $$PWD/../dde-file-manager-lib/interfaces/plugins \
                $$PWD/../dde-file-manager-lib/io

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/release -ldde-file-manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/debug -ldde-file-manager
else:unix: LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

CONFIG(debug, debug|release) {
    DEPENDPATH += $$PWD/../dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../dde-file-manager-lib
}

#安全加固
QMAKE_CXXFLAGS += -fstack-protector-all
QMAKE_LFLAGS += -z now -pie -fPIE
isEqual(ARCH, mips64) | isEqual(ARCH, mips32){
    QMAKE_LFLAGS += -z noexecstack -z relro
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
    presenter/dfmsocketinterface.cpp \
    view/watermaskframe.cpp \
    dbus/dbusdock.cpp \
    view/desktopitemdelegate.cpp \
    presenter/gridcore.cpp \
    dbus/dbusmonitor.cpp \
    screen/abstractscreen.cpp \
    screen/abstractscreenmanager.cpp \
    screen/screenmanager.cpp \
    screen/screenobject.cpp \
    view/backgroundmanager.cpp \
    screen/screenhelper.cpp \
    view/backgroundwidget.cpp \
    screen/screenmanagerwayland.cpp \
    screen/screenobjectwayland.cpp \
    dbus/licenceInterface.cpp \
    view/canvasviewmanager.cpp \
    presenter/deepinlicensehelper.cpp



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
    presenter/dfmsocketinterface.h \
    view/watermaskframe.h \
    dbus/dbusdock.h \
    view/desktopitemdelegate.h \
    presenter/gridcore.h \
    deventfilter.h \
    dbus/dbusmonitor.h \
    dbus/licenceInterface.h \
    screen/abstractscreen.h \
    screen/abstractscreenmanager.h \
    screen/abstractscreenmanager_p.h \
    screen/screenmanager.h \
    screen/screenobject.h \
    view/backgroundmanager.h \
    screen/screenhelper.h \
    view/backgroundwidget.h \
    screen/screenmanagerwayland.h \
    screen/screenobjectwayland.h \
    view/canvasviewmanager.h \
    accessibility/ac-desktop-define.h \
    accessibility/acobjectlist.h \
    desktopprivate.h \
    presenter/dfmsocketinterface_p.h \
    presenter/deepinlicensehelper.h

RESOURCES +=

# Automating generation .qm files from .ts files

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
    !system($$PWD/translate_ts2desktop.sh): error("Failed to generate translation")
}

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
                $$PWD/translations/$${TARGET}_zh_CN.ts

include($$PWD/development.pri)
