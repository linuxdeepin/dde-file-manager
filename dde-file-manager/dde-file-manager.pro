#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

include(../common/common.pri)
include(dbus/dbus.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += network

isEmpty(TARGET) {
    TARGET = $$ProjectName
}

TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += gio-unix-2.0
CONFIG(release, release|debug) {
    PKGCONFIG += dtkbase dtkwidget dtkutil
} else {
    PKGCONFIG += dtkbase dtkwidget dtkutil
}

DEFINES += QMAKE_TARGET=\\\"$$TARGET\\\" QMAKE_VERSION=\\\"$$VERSION\\\"

DEFINES += QT_MESSAGELOGCONTEXT

isEmpty(QMAKE_ORGANIZATION_NAME) {
    DEFINES += QMAKE_ORGANIZATION_NAME=\\\"deepin\\\"
}

isEmpty(PREFIX){
    PREFIX = /usr
}



CONFIG(release, debug|release) {
    !system($$PWD/translate_ts2desktop.sh): error("Failed to generate translation")
}

CONFIG(debug, debug|release) {
#    LIBS += -lprofiler
#    DEFINES += ENABLE_PPROF
}

SOURCES += \
    main.cpp \
    filemanagerapp.cpp \
    logutil.cpp \
    singleapplication.cpp \
    commandlinemanager.cpp

INCLUDEPATH += $$PWD/../dde-file-manager-lib $$PWD/.. \
               $$PWD/../utils \
               $$PWD/../dde-file-manager-lib/interfaces

BINDIR = $$PREFIX/bin
DEFINES += APPSHAREDIR=\\\"$$PREFIX/share/$$TARGET\\\"

DEFINES += AUTOMOUNT

target.path = $$BINDIR

desktop.path = $${PREFIX}/share/applications/
isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    desktop.files = $$PWD/mips/$${TARGET}.desktop
}else{
    desktop.files = $${TARGET}.desktop
}

dde-xdg-user-dirs-update.path = $$BINDIR
dde-xdg-user-dirs-update.files = $$PWD/dde-xdg-user-dirs-update.sh

policy.path = $${PREFIX}/share/polkit-1/actions/
policy.files = pkexec/com.deepin.pkexec.dde-file-manager.policy

pkexec.path = /usr/bin/
pkexec.files = pkexec/dde-file-manager-pkexec

propertyDialogShell.path = /usr/bin/
propertyDialogShell.files = dde-property-dialog

INSTALLS += target desktop dde-xdg-user-dirs-update policy pkexec propertyDialogShell

isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    dde-mips-shs.path = $$BINDIR
    dde-mips-shs.files = $$PWD/mips/dde-computer.sh \
                         $$PWD/mips/dde-trash.sh \
                         $$PWD/mips/file-manager.sh

    dde-file-manager-autostart.path = /etc/xdg/autostart
    dde-file-manager-autostart.files = $$PWD/mips/dde-file-manager-autostart.desktop dde-file-manager-dialog-autostart.desktop
    INSTALLS += dde-mips-shs dde-file-manager-autostart
}else{
    autostart.path = /etc/xdg/autostart
    autostart.files = dde-file-manager-xdg-autostart.desktop dde-file-manager-dialog-autostart.desktop
    INSTALLS += autostart
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/release -ldde-file-manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/debug -ldde-file-manager
else:unix: LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

CONFIG(debug, debug|release) {
    DEPENDPATH += $$PWD/../dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../dde-file-manager-lib
}

HEADERS += \
    filemanagerapp.h \
    logutil.h \
    singleapplication.h \
    commandlinemanager.h

DISTFILES += \
    mips/dde-file-manager-autostart.desktop \
    mips/dde-file-manager.desktop \
    mips/dde-computer.sh \
    mips/dde-trash.sh \
    mips/file-manager.sh \
    generate_translations.sh

TRANSLATIONS += $$PWD/translations/$${TARGET}-app.ts \
    $$PWD/translations/$${TARGET}-app_zh_CN.ts

translations.path = $$APPSHAREDIR/translations
translations.files = translations/*.qm

INSTALLS += translations

CONFIG(release, debug|release) {
    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
}
