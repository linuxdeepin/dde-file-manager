#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += network

isEmpty(TARGET) {
    TARGET = dde-file-manager
}

TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkbase dtkwidget dtkutil

isEmpty(VERSION) {
    VERSION = 1.3
}

DEFINES += QMAKE_TARGET=\\\"$$TARGET\\\" QMAKE_VERSION=\\\"$$VERSION\\\"

isEmpty(QMAKE_ORGANIZATION_NAME) {
    DEFINES += QMAKE_ORGANIZATION_NAME=\\\"deepin\\\"
}

isEmpty(PREFIX){
    PREFIX = /usr
}

# Automating generation .qm files from .ts files
# system($$PWD/desktop/translate_generation.sh)

CONFIG(debug, debug|release) {
#    LIBS += -lprofiler
#    DEFINES += ENABLE_PPROF
}

SOURCES += \
    main.cpp \
    filemanagerapp.cpp \
    logutil.cpp \
    singleapplication.cpp

INCLUDEPATH += $$PWD/../dde-file-manager-lib $$PWD/.. \
               $$PWD/../utils \
               $$PWD/../dde-file-manager-lib/interfaces

BINDIR = $$PREFIX/bin
DEFINES += APPSHAREDIR=\\\"$$PREFIX/share/$$TARGET\\\"

isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    DEFINES += AUTO_RESTART_DEAMON
}


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
    dde-file-manager-autostart.files = $$PWD/mips/dde-file-manager-autostart.desktop
    INSTALLS += dde-mips-shs dde-file-manager-autostart
}else{
    xdg_autostart.path = /etc/xdg/autostart
    xdg_autostart.files = dde-file-manager-xdg-autostart.desktop
    INSTALLS += xdg_autostart
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/release -ldde-file-manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dde-file-manager-lib/debug -ldde-file-manager
else:unix: LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

HEADERS += \
    filemanagerapp.h \
    logutil.h \
    singleapplication.h

DISTFILES += \
    mips/dde-file-manager-autostart.desktop \
    mips/dde-file-manager.desktop \
    mips/dde-computer.sh \
    mips/dde-trash.sh \
    mips/file-manager.sh \
    generate_translations.sh
