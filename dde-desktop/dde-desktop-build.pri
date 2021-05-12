include(../common/common.pri)

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

CONFIG(TABLET_ENV) {
    PKGCONFIG += dtkwidget5.5 dtkgui5.5 dtkcore5.5
} else {
    PKGCONFIG += dtkwidget dtkgui dtkcore
}

load(dtk_qmake)

# add computer/trash icon on professional system
deepin_professional: DEFINES += DDE_COMPUTER_TRASH

!isEmpty(DISABLE_SCREENSAVER) {
    DEFINES += DISABLE_SCREENSAVER
}

!isEmpty(DISABLE_WALLPAPER_CAROUSEL) {
    DEFINES += DISABLE_WALLPAPER_CAROUSEL
}
