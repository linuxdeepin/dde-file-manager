TEMPLATE  = subdirs

include(./src/common/common.pri)

CONFIG += ordered

SUBDIRS += \
    src/dde-file-manager-lib/dde-file-manager-lib.pro\
    src/dde-desktop/test-dde-desktop.pro \
    src/dde-dock-plugins/test-dde-dock-plugins.pro \
    src/dde-file-manager-daemon/test-dde-file-manager-daemon.pro \
    tests/dde-file-manager/test-dde-file-manager.pro \
    src/dde-file-manager-plugins/test-dde-file-manager-plugins.pro \
    src/dde-file-manager-lib/test-dde-file-manager-lib.pro \
    src/dde-file-thumbnail-tool/test-dde-file-thumbnail-tool.pro \
    src/deepin-anything-server-plugins/test-deepin-anything-server-plugins.pro \
