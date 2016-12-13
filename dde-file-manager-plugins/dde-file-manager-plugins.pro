TEMPLATE = subdirs

include(../common/common.pri)

isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    SUBDIRS += \
        showfile
}
