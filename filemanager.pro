TEMPLATE  = subdirs

include(./src/common/common.pri)

isEmpty(PREFIX){
    PREFIX = /usr
}

SUBDIRS  += \
    dde-file-manager \
    dde-file-manager-daemon \
    dde-file-manager-lib \
    dde-file-manager-plugins \
    dde-dock-plugins \
    dde-desktop \
    dde-select-dialog-x11 \
    dde-select-dialog-wayland \
    dde-file-thumbnail-tool \
    gschema

!CONFIG(DISABLE_ANYTHING) {
    message("Deepin Anything server plugin enabled for" $$ARCH)
    SUBDIRS += deepin-anything-server-plugins
}

dde-file-manager.subdir = src/dde-file-manager
dde-file-manager-lib.subdir = src/dde-file-manager-lib
dde-file-manager-daemon.subdir = src/dde-file-manager-daemon
dde-file-manager-plugins.subdir = src/dde-file-manager-plugins
dde-dock-plugins.subdir = src/dde-dock-plugins
dde-desktop.subdir = src/dde-desktop
dde-file-thumbnail-tool.subdir = src/dde-file-thumbnail-tool
gschema.subdir = src/gschema
deepin-anything-server-plugins.subdir = src/deepin-anything-server-plugins
dde-select-dialog-x11.subdir = src/dde-select-dialog-x11
dde-select-dialog-wayland.subdir = src/dde-select-dialog-wayland

dde-file-manager.depends = dde-file-manager-lib
dde-dock-plugins.depends = dde-file-manager-lib
dde-desktop.depends = dde-file-manager-lib
dde-file-manager-daemon.depends = dde-file-manager-lib
deepin-anything-server-plugins.depends = dde-file-manager-lib
#dde-sharefiles.depends = dde-file-manager-lib
dde-select-dialog-x11.depends = dde-file-manager-lib
dde-select-dialog-wayland.depends = dde-file-manager-lib
