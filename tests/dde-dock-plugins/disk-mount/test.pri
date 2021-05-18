#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

INCLUDEPATH += $$PWD


SOURCES += \
    $$PWD/main.cpp \
    $$PWD/cases/ut_dattachedudisks2device.cpp \
    $$PWD/cases/ut_dattachedvfsdevice.cpp \
    $$PWD/cases/ut_diskcontrolitem.cpp \
    $$PWD/cases/ut_diskpluginitem.cpp \
    $$PWD/cases/ut_diskmountplugin.cpp \
    $$PWD/cases/ut_diskcontrolwidget.cpp \
    $$PWD/cases/ut_mock_stub_disk_gio.cpp \
    $$PWD/cases/ut_mock_stub_diskdevice.cpp

HEADERS += \
    $$PWD/cases/ut_mock_dattacheddeviceinterface.h \
    $$PWD/cases/ut_mock_pluginproxyinterface.h \
    $$PWD/cases/ut_mock_stub_diskdevice.h \
    $$PWD/cases/ut_mock_stub_disk_gio.h \
    $$PWD/cases/ut_mock_stub_common_define.h

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
