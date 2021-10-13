#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

PRJ_FOLDER = $$PWD/../../
SRC_FOLDER = $$PRJ_FOLDER/src
LIB_DFM_SRC_FOLDER = $$SRC_FOLDER/dde-file-manager-lib

include($$SRC_FOLDER/common/common.pri)

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
PKGCONFIG += x11 libsecret-1 gio-unix-2.0 poppler-cpp dtkwidget dtkgui udisks2-qt5 disomaster gio-qt libcrypto Qt5Xdg xcb xcb-ewmh xcb-shape dframeworkdbus polkit-agent-1 polkit-qt5-1
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


include($$SRC_FOLDER/dialogs/dialogs.pri)
include($$SRC_FOLDER/utils/utils.pri)
include($$SRC_FOLDER/chinese2pinyin/chinese2pinyin.pri)
include($$SRC_FOLDER/fileoperations/fileoperations.pri)
include($$LIB_DFM_SRC_FOLDER/deviceinfo/deviceinfo.pri)
include($$LIB_DFM_SRC_FOLDER/dbusinterface/dbusinterface.pri)
include($$SRC_FOLDER/usershare/usershare.pri)
include($$SRC_FOLDER/dde-file-manager-plugins/plugininterfaces/plugininterfaces.pri)
include($$LIB_DFM_SRC_FOLDER/tag/tag.pri)
include($$LIB_DFM_SRC_FOLDER/mediainfo/mediainfo.pri)
include($$LIB_DFM_SRC_FOLDER/vault/vault.pri)
include($$LIB_DFM_SRC_FOLDER/fulltextsearch/fulltextsearch.pri)
include($$LIB_DFM_SRC_FOLDER/log/log.pri)

isEqual(ARCH, sw_64){
#    isEqual(ENABLE_SW_LABLE, YES){
        DEFINES += SW_LABEL
        include($$LIB_DFM_SRC_FOLDER/sw_label/sw_label.pri)
#    }
}
include($$LIB_DFM_SRC_FOLDER/io/io.pri)
include($$LIB_DFM_SRC_FOLDER/interfaces/vfs/vfs.pri)
include($$LIB_DFM_SRC_FOLDER/interfaces/customization/customization.pri)
include($$LIB_DFM_SRC_FOLDER/src.pri)

isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) | isEqual(ARCH, aarch64) | isEqual(ARCH, loongarch64) {
    include($$LIB_DFM_SRC_FOLDER/search/dfsearch.pri)
}
APPSHAREDIR = $$PREFIX/share/$$TARGET
ICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/dde-file-manager
} else {
    includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager
}

includes.files += $$LIB_DFM_SRC_FOLDER/interfaces/*.h $$LIB_DFM_SRC_FOLDER/interfaces/plugins/*.h

includes_private.path = $${includes.path}/private
includes_private.files += $$LIB_DFM_SRC_FOLDER/interfaces/private/*.h

isEmpty(INCLUDE_INSTALL_DIR) {
    gvfs_includes.path = $$PREFIX/include/dde-file-manager/gvfs
} else {
    gvfs_includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager/gvfs
}

gvfs_includes.files += $$LIB_DFM_SRC_FOLDER/gvfs/*.h

isEmpty(INCLUDE_INSTALL_DIR) {
    plugin_includes.path = $$PREFIX/include/dde-file-manager/dde-file-manager-plugins
} else {
    plugin_includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager/dde-file-manager-plugins
}

plugin_includes.files += $$SRC_FOLDER/dde-file-manager-plugins/plugininterfaces/menu/*.h
plugin_includes.files += $$SRC_FOLDER/dde-file-manager-plugins/plugininterfaces/preview/*.h
plugin_includes.files += $$SRC_FOLDER/dde-file-manager-plugins/plugininterfaces/view/*.h


templateFiles.path = $$APPSHAREDIR/templates

mimetypeFiles.path = $$APPSHAREDIR/mimetypes
mimetypeFiles.files += \
    $$LIB_DFM_SRC_FOLDER/mimetypes/archive.mimetype \
    $$LIB_DFM_SRC_FOLDER/mimetypes/text.mimetype \
    $$LIB_DFM_SRC_FOLDER/mimetypes/video.mimetype \
    $$LIB_DFM_SRC_FOLDER/mimetypes/audio.mimetype \
    $$LIB_DFM_SRC_FOLDER/mimetypes/image.mimetype \
    $$LIB_DFM_SRC_FOLDER/mimetypes/executable.mimetype \
    $$LIB_DFM_SRC_FOLDER/mimetypes/backup.mimetype

mimetypeAssociations.path = $$APPSHAREDIR/mimetypeassociations
mimetypeAssociations.files += \
    $$LIB_DFM_SRC_FOLDER/mimetypeassociations/mimetypeassociations.json


include($$LIB_DFM_SRC_FOLDER/settings_dialog_json.pri)

TEMPLATE = app
CONFIG += console

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

# resolve .gcda merge issue.
system(find $$OUT_PWD -name "*.gcda" -print0 | xargs -0 rm -f)

#include($$PRJ_FOLDER/3rdparty/googletest/gtest_dependency.pri)
include($$PRJ_FOLDER/3rdparty/cpp-stub/stub.pri)
INCLUDEPATH += $$PRJ_FOLDER/3rdparty/stubext
LIBS += -lgcov

unix {
    LIBS += -lgtest -lgmock
}

HEADERS += \
    dialogs/burnoptdialog_p.h

include(test.pri)
