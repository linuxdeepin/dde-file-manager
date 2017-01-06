TEMPLATE  = subdirs

include(common/common.pri)

SUBDIRS  += \
    dde-file-manager \
    dde-file-manager-daemon \
    dde-file-manager-lib \
    dde-file-manager-plugins \
    usb-device-formatter

dde-file-manager.depends = dde-file-manager-lib
