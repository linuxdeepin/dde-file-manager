TEMPLATE  = subdirs

SUBDIRS  += \
    dde-file-manager \
    dde-file-manager-daemon \
    dde-file-manager-lib \
    dde-file-manager-plugins \
    dde-dock-plugins\
    dde-desktop

isEqual(BUILD_MINIMUM, YES){

}else{
    SUBDIRS += usb-device-formatter
#    dde-advanced-property-plugin \
#    dde-sharefiles
}


dde-file-manager.depends = dde-file-manager-lib
dde-dock-plugins.depends = dde-file-manager-lib
dde-desktop.depends = dde-file-manager-lib
#dde-sharefiles.depends = dde-file-manager-lib
