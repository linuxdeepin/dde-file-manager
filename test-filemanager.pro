TEMPLATE  = subdirs

include(./common/common.pri)

CONFIG += ordered

SUBDIRS += \
    dde-file-manager-lib/dde-file-manager-lib-depend.pro\
    dde-file-manager-lib/test-dde-file-manager-lib.pro\
    dde-desktop/test-dde-desktop.pro \
    dde-dock-plugins/test-dde-dock-plugins.pro
