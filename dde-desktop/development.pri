isEmpty(PREFIX){
    PREFIX = /usr
}

target.path = $${PREFIX}/bin/

desktop_files.path = /usr/share/applications/
isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    desktop_files.files = $$PWD/data/applications/mips/dde-home.desktop \
                    $$PWD/data/applications/dde-computer.desktop \
                    $$PWD/data/applications/dde-trash.desktop
} else {
    desktop_files.files = $$PWD/data/applications/dde-home.desktop \
                    $$PWD/data/applications/dde-computer.desktop \
                    $$PWD/data/applications/dde-trash.desktop
}

services.path = /usr/share/dbus-1/services
services.files = $$PWD/data/com.deepin.dde.desktop.service

translations.path = $${PREFIX}/share/$${TARGET}/translations
translations.files = translations/*.qm

INSTALLS += target desktop_files services translations

GitVersion = $$system(git rev-parse HEAD)
DEFINES += GIT_VERSION=\\\"$$GitVersion\\\"

