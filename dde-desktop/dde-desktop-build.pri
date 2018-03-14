DEFINES += QT_MESSAGELOGCONTEXT

EDITION=COMMUNITY
ARCH = $$QMAKE_HOST.arch
isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    EDITION=RACCOON
}

isEqual(EDITION, RACCOON) {
    DEFINES += DDE_COMPUTER_TRASH
}

DEFINES += DDE_DBUS_DISPLAY

CONFIG(release, release|debug) {
    PKGCONFIG += dtkwidget
} else {
    PKGCONFIG += dtkwidget
}

load(deepin_qt)

# add computer/trash icon on professional system
deepin_professional: DEFINES += DDE_COMPUTER_TRASH
