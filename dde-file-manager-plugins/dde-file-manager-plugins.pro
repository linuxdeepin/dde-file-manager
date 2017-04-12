TEMPLATE = subdirs

include(../common/common.pri)

IS_TEST_VIEW_PLUGIN = False

isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
#    SUBDIRS += \
#        showfile
}

isEqual(IS_TEST_VIEW_PLUGIN, True){
    SUBDIRS += \
        pluginView
}

SUBDIRS += \
    pluginPreview
