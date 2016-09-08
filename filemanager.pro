TEMPLATE  = subdirs

SUBDIRS  += \
    dde-file-manager \
    dde-file-manager-daemon \
    dde-file-manager-lib

dde-file-manager.depends = dde-file-manager-lib
