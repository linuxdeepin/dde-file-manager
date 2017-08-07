TEMPLATE  = subdirs

SUBDIRS  += \
    dde-file-manager \
    dde-file-manager-daemon \
    dde-file-manager-lib \
    dde-file-manager-plugins \
    dde-dock-plugins\
    dde-desktop\
    usb-device-formatter \
#    dde-advanced-property-plugin \
    usb-device-formatter
#    dde-sharefiles

dde-file-manager.depends = dde-file-manager-lib
dde-dock-plugins.depends = dde-file-manager-lib
dde-desktop.depends = dde-file-manager-lib
#dde-sharefiles.depends = dde-file-manager-lib
