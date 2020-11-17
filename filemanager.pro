TEMPLATE  = subdirs

include(./common/common.pri)

isEmpty(PREFIX){
    PREFIX = /usr
}

SUBDIRS  += \
    dde-file-manager \
    dde-file-manager-daemon \
    dde-file-manager-lib \
    dde-file-manager-plugins \
    dde-dock-plugins\
    dde-desktop \
    dde-file-thumbnail-tool \
    gschema

!CONFIG(DISABLE_ANYTHING) {
    message("Deepin Anything server plugin enabled for" $$ARCH)
    SUBDIRS += deepin-anything-server-plugins
}

dde-file-manager.depends = dde-file-manager-lib
dde-dock-plugins.depends = dde-file-manager-lib
dde-desktop.depends = dde-file-manager-lib
dde-file-manager-daemon.depends = dde-file-manager-lib
deepin-anything-server-plugins.depends = dde-file-manager-lib
#dde-sharefiles.depends = dde-file-manager-lib
