SOURCES += \
    $$PWD/dde-wallpaper-chooser/ut-frame-test.cpp \
    $$PWD/dde-wallpaper-chooser/ut-button-test.cpp\
    $$PWD/dde-wallpaper-chooser/ut-waititem-test.cpp \
    $$PWD/dde-wallpaper-chooser/ut-wallpaperitem-test.cpp \
    $$PWD/dde-wallpaper-chooser/ut-wallpaperlist-test.cpp \
    $$PWD/dde-wallpaper-chooser/ut-thumbnailmanager-test.cpp \
    $$PWD/dde-wallpaper-chooser/ut-checkbox-test.cpp \
    $$PWD/dde-wallpaper-chooser/ut-screensavercontrol-test.cpp \
    $$PWD/view/ut-canvasviewhelper-test.cpp \
    $$PWD/view/ut-watermaskframe-test.cpp  \
    $$PWD/view/ut_backgroundmanager_test.cpp \
    $$PWD/view/ut_backgroundwidget_test.cpp \
    $$PWD/view/ut_canvasgridview_test.cpp \
    $$PWD/view/ut_canvasviewmanager_test.cpp\
    $$PWD/ut-desktop-test.cpp \
    $$PWD/util/ut-util-test.cpp \
    $$PWD/util/ut-desktopinfo-test.cpp \
    $$PWD/config/ut-config-test.cpp\
    $$PWD/global/ut_coorinate_test.cpp \
    $$PWD/global/ut_singleton_test.cpp \
    $$PWD/presenter/ut-presenter-test.cpp \
    $$PWD/presenter/ut-gridmanager-test.cpp \
    $$PWD/presenter/ut-gridcore-test.cpp \
    $$PWD/model/ut-dfileselectionmodel.cpp \
    $$PWD/screen/ut-screenmanager-test.cpp \
    #$$PWD/presenter/ut-dfmsocketinterface-test.cpp\
    $$PWD/dde-wallpaper-chooser/ut-autoactivatewindow.cpp

#屏蔽热区
#    $$PWD/dde-zone/ut-pushbuttonlist-test.cpp \
#    $$PWD/dde-zone/ut-hotzone-test.cpp \
#屏蔽dbus接口
#    $$PWD/dbus/filedialog/ut_dbusfiledialogmanager_test.cpp \
#    $$PWD/dbus/filedialog/ut_dbusfiledialoghandle_test.cpp \
#    $$PWD/dbus/filemanager1/ut_dbusfilemanager_test.cpp \
#    $$PWD/dde-wallpaper-chooser/ut-deepin_wm-test.cpp   \

!CONFIG(DISABLE_TSAN_TOOL) {
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
    contains(DEFINES, TSAN_THREAD){
       QMAKE_CXXFLAGS+="-fsanitize=thread"
       QMAKE_CFLAGS+="-fsanitize=thread"
       QMAKE_LFLAGS+="-fsanitize=thread"
    } else {
       QMAKE_CXXFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_CFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_LFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
    }
}
