TEMPLATE  = subdirs

include(./src/common/common.pri)

CONFIG += ordered


SUBDIRS += \
    src/dde-file-manager-lib/dde-file-manager-lib.pro\
    tests/dde-desktop/test-dde-desktop.pro \
    tests/dde-dock-plugins/test-dde-dock-plugins.pro \
    tests/dde-file-manager/test-dde-file-manager.pro \
    tests/dde-file-manager-daemon/test-dde-file-manager-daemon.pro \
    tests/dde-file-manager-plugins/test-dde-file-manager-plugins.pro \
    tests/dde-file-manager-lib/test-dde-file-manager-lib.pro \
    tests/dde-file-thumbnail-tool/test-dde-file-thumbnail-tool.pro

isEqual(ARCH, x86_64) | isEqual(ARCH, i686){
    SUBDIRS += \
    tests/deepin-anything-server-plugins/test-deepin-anything-server-plugins.pro
}
