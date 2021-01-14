TEMPLATE = subdirs

include(../../src/common/common.pri)

isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
#    SUBDIRS += \
#        showfile
}

SUBDIRS += \
    pluginPreview/test-pluginPreview.pro \
    pluginView/test-pluginView.pro
