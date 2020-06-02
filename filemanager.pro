TEMPLATE  = subdirs

include(./common/common.pri)

CONFIG+=BUILD_MINIMUM DISABLE_ANYTHING

SUBDIRS  += \
    dde-file-manager \
    dde-file-manager-daemon \
    dde-desktop \
    dde-file-manager-lib \
    gschema


!CONFIG(BUILD_MINIMUM) {
message("fully build dde-file-manager")
SUBDIRS += \
    dde-file-manager-plugins \
    dde-dock-plugins \
    dde-file-thumbnail-tool
   }

!CONFIG(DISABLE_ANYTHING) {
    message("Deepin Anything server plugin enabled for" $$ARCH)
    SUBDIRS += deepin-anything-server-plugins
}

dde-file-manager.depends = dde-file-manager-lib
dde-dock-plugins.depends = dde-file-manager-lib
dde-desktop.depends = dde-file-manager-lib
dde-file-manager-daemon.depends = dde-file-manager-lib
deepin-anything-server-plugins.depends = dde-file-manager-lib
