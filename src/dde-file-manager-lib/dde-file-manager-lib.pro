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
LIBS	 += -lKF5Codecs

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += widgets-private

TARGET = $$ProjectName

TEMPLATE = lib
CONFIG += create_pc create_prl no_install_prl

DEFINES += QMAKE_TARGET=\\\"$$TARGET\\\" QMAKE_VERSION=\\\"$$VERSION\\\"

# sp3 不稳定的feature，使用宏 UNSTABLE_FEATURE_ENABLE 屏蔽，待稳定后放开
#DEFINES += SP3_UNSTABLE_FEATURE_ENABLE

isEmpty(QMAKE_ORGANIZATION_NAME) {
    DEFINES += QMAKE_ORGANIZATION_NAME=\\\"deepin\\\"
}

isEmpty(PREFIX){
    PREFIX = /usr
}

CONFIG += c++11 link_pkgconfig
PKGCONFIG += libsecret-1 gio-unix-2.0 poppler-cpp dtkwidget dtkgui udisks2-qt5 disomaster gio-qt libcrypto Qt5Xdg dframeworkdbus
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

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

# 获取标签系统设置
AC_FUNC_ENABLE = true
#AC_FUNC_ENABLE = $$(ENABLE_AC_FUNC)
# 检查集成测试标签
equals( AC_FUNC_ENABLE, true ){
    DEFINES += ENABLE_ACCESSIBILITY
    message("lib-dde-file-manager enabled accessibility function with set: " $$AC_FUNC_ENABLE)
}

include(../dialogs/dialogs.pri)
include(../utils/utils.pri)
include(../chinese2pinyin/chinese2pinyin.pri)
include(../fileoperations/fileoperations.pri)
include(deviceinfo/deviceinfo.pri)
include(devicemanagement/devicemanagement.pri)
include(dbusinterface/dbusinterface.pri)
include(../usershare/usershare.pri)
include(../dde-file-manager-plugins/plugininterfaces/plugininterfaces.pri)
include(tag/tag.pri)
include(mediainfo/mediainfo.pri)
include(vault/vault.pri)
include(log/log.pri)

isEqual(ARCH, sw_64){
    DEFINES += SW_LABEL
    include(./sw_label/sw_label.pri)
}else{
    include(fulltextsearch/fulltextsearch.pri)
    DEFINES += FULLTEXTSEARCH_ENABLE
    TR_EXCLUDE += /usr/include/boost/ \
              $$PWD/fulltextsearch/*
}
include(io/io.pri)
include(interfaces/vfs/vfs.pri)
include(interfaces/customization/customization.pri)
include(src.pri)

isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) | isEqual(ARCH, aarch64){
    include(search/dfsearch.pri)
}

APPSHAREDIR = $$PREFIX/share/$$TARGET
ICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

win32* {
    DEFINES += STATIC_LIB
    CONFIG += staticlib
    LIB_DIR =
}

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$[QT_INSTALL_LIBS]
} else {
    target.path = $$LIB_INSTALL_DIR
}

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

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_NAME = dde-file-manager
QMAKE_PKGCONFIG_DESCRIPTION = DDE File Manager Header Files
QMAKE_PKGCONFIG_INCDIR = $$includes.path

templateFiles.path = $$APPSHAREDIR/templates

isEqual(BUILD_MINIMUM, YES){
    templateFiles.files = skin/templates/newTxt.txt
}else{
    templateFiles.files = skin/templates/newDoc.doc \
        skin/templates/newExcel.xls \
        skin/templates/newPowerPoint.ppt \
        skin/templates/newDoc.wps \
        skin/templates/newExcel.et \
        skin/templates/newPowerPoint.dps \
        skin/templates/newTxt.txt
}

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

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
    !system($$PWD/update_translations.sh): error("Failed to generate translation")
#    DEFINES += QT_NO_DEBUG_OUTPUT
}

translations.path = $$APPSHAREDIR/translations
translations.files = translations/*.qm

icon.path = $$ICONDIR
icon.files = skin/images/$${TARGET}.svg

defaultConfig.path = $$APPSHAREDIR/config
defaultConfig.files = configure/default-view-states.json

# readme file for create oem-menuextension directory
readmefile.path = $$PREFIX/share/deepin/$$TARGET/oem-menuextensions
readmefile.files = plugins/.readme

INSTALLS += target templateFiles translations mimetypeFiles mimetypeAssociations \
 icon includes includes_private gvfs_includes plugin_includes defaultConfig readmefile policy

DISTFILES += \
    mimetypeassociations/mimetypeassociations.json \
    confirm/deepin-vault-authenticateProxy \
    policy/com.deepin.pkexec.deepin-vault-authenticateProxy.policy

include($$PWD/settings_dialog_json.pri)
