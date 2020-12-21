#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

include(../common/common.pri)

QT       += core gui svg dbus x11extras concurrent multimedia dbus xml KCodecs network
#private
QT       += gui-private

#for qtest
QT       += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += widgets-private

TARGET = test-dde-file-manager-lib

CONFIG += create_pc create_prl no_install_prl

DEFINES += QMAKE_TARGET=\\\"$$TARGET\\\" QMAKE_VERSION=\\\"$$VERSION\\\"

isEmpty(QMAKE_ORGANIZATION_NAME) {
    DEFINES += QMAKE_ORGANIZATION_NAME=\\\"deepin\\\"
}

isEmpty(PREFIX){
    PREFIX = /usr
}

CONFIG += c++11 link_pkgconfig
PKGCONFIG += x11 libsecret-1 gio-unix-2.0 poppler-cpp dtkwidget dtkgui udisks2-qt5 disomaster gio-qt libcrypto Qt5Xdg xcb xcb-ewmh xcb-shape dframeworkdbus
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT
DEFINES += BLUETOOTH_ENABLE

CONFIG(DISABLE_FFMPEG) | isEqual(BUILD_MINIMUM, YES) {
    DEFINES += DISABLE_FFMEPG
}

# BUILD_MINIMUM for live system
isEqual(BUILD_MINIMUM, YES){
    DEFINES += DFM_MINIMUM
}

CONFIG(DISABLE_ANYTHING) {
    message("Quick search and tag support disabled dut to Anything support disabled.")
    DEFINES += DISABLE_QUICK_SEARCH
    DEFINES += DISABLE_TAG_SUPPORT
}

include(../dialogs/dialogs.pri)
include(../utils/utils.pri)
include(../chinese2pinyin/chinese2pinyin.pri)
include(../fileoperations/fileoperations.pri)
include(deviceinfo/deviceinfo.pri)
include(dbusinterface/dbusinterface.pri)
include(../usershare/usershare.pri)
include(../dde-file-manager-plugins/plugininterfaces/plugininterfaces.pri)
include(tag/tag.pri)
include(mediainfo/mediainfo.pri)
include(vault/vault.pri)
include(fulltextsearch/fulltextsearch.pri)
include(log/log.pri)

isEqual(ARCH, sw_64){
#    isEqual(ENABLE_SW_LABLE, YES){
        DEFINES += SW_LABEL
        include(./sw_label/sw_label.pri)
#    }
}
include(io/io.pri)
include(interfaces/vfs/vfs.pri)
include(interfaces/customization/customization.pri)
include(src.pri)

APPSHAREDIR = $$PREFIX/share/$$TARGET
ICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/dde-file-manager
} else {
    includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager
}

includes.files += $$PWD/interfaces/*.h $$PWD/interfaces/plugins/*.h

includes_private.path = $${includes.path}/private
includes_private.files += $$PWD/interfaces/private/*.h

isEmpty(INCLUDE_INSTALL_DIR) {
    gvfs_includes.path = $$PREFIX/include/dde-file-manager/gvfs
} else {
    gvfs_includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager/gvfs
}

gvfs_includes.files += $$PWD/gvfs/*.h

isEmpty(INCLUDE_INSTALL_DIR) {
    plugin_includes.path = $$PREFIX/include/dde-file-manager/dde-file-manager-plugins
} else {
    plugin_includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager/dde-file-manager-plugins
}

plugin_includes.files += $$PWD/../dde-file-manager-plugins/plugininterfaces/menu/*.h
plugin_includes.files += $$PWD/../dde-file-manager-plugins/plugininterfaces/preview/*.h
plugin_includes.files += $$PWD/../dde-file-manager-plugins/plugininterfaces/view/*.h


templateFiles.path = $$APPSHAREDIR/templates

mimetypeFiles.path = $$APPSHAREDIR/mimetypes
mimetypeFiles.files += \
    mimetypes/archive.mimetype \
    mimetypes/text.mimetype \
    mimetypes/video.mimetype \
    mimetypes/audio.mimetype \
    mimetypes/image.mimetype \
    mimetypes/executable.mimetype \
    mimetypes/backup.mimetype

mimetypeAssociations.path = $$APPSHAREDIR/mimetypeassociations
mimetypeAssociations.files += \
    mimetypeassociations/mimetypeassociations.json


include($$PWD/settings_dialog_json.pri)

TEMPLATE = app
CONFIG += console

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

# resolve .gcda merge issue.
system(find $$OUT_PWD -name "*.gcda" -print0 | xargs -0 rm -f)

include(../third-party/googletest/gtest_dependency.pri)
include(tests/test.pri)
include(../third-party/cpp-stub/stub.pri)
INCLUDEPATH += $$top_srcdir/third-party/stubext
LIBS += -lgcov
