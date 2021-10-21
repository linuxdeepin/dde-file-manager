SOURCES += $$PWD/frame.cpp \
    $$PWD/wallpaperlist.cpp \
    $$PWD/wallpaperitem.cpp \
    $$PWD/thumbnailmanager.cpp \
    $$PWD/button.cpp \
    $$PWD/dbus/deepin_wm.cpp \
    $$PWD/checkbox.cpp \
    $$PWD/screensavercontrol.cpp \
    $$PWD/waititem.cpp \
    $$PWD/utils/autoactivatewindow.cpp

HEADERS  += $$PWD/frame.h \
    $$PWD/wallpaperlist.h \
    $$PWD/wallpaperitem.h \
    $$PWD/constants.h \
    $$PWD/thumbnailmanager.h \
    $$PWD/button.h \
    $$PWD/dbus/deepin_wm.h \
    $$PWD/checkbox.h \
    $$PWD/screensavercontrol.h \
    $$PWD/waititem.h \
    $$PWD/utils/autoactivatewindow.h \
    $$PWD/utils/autoactivatewindow_p.h

INCLUDEPATH += $$_PRO_FILE_PWD_/view

RESOURCES += \
    $$PWD/images.qrc

isEmpty(DISABLE_SCREENSAVER) {
    dbus_screensaver.files = $$PWD/dbus/com.deepin.ScreenSaver.xml
    DBUS_INTERFACES += dbus_screensaver
}

isEmpty(DISABLE_WALLPAPER_CAROUSEL) {
    dbus_appearance.files = $$PWD/dbus/com.deepin.daemon.Appearance.xml
    DBUS_INTERFACES += dbus_appearance
}
