TEMPLATE = subdirs

SUBDIRS += \
    dde-image-preview-plugin/test-dde-image-preview-plugin.pro \
    dde-pdf-preview-plugin/test-dde-pdf-preview-plugin.pro \
    dde-text-preview-plugin/test-dde-text-preview-plugin.pro \
    dde-music-preview-plugin/test-dde-music-preview-plugin.pro

ARCH = $$QMAKE_HOST.arch

!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    !isEqual(ARCH, sw_64):!isEqual(ARCH, mips64):!isEqual(ARCH, mips32):!isEqual(ARCH, loongarch64)  {
        SUBDIRS += dde-video-preview-plugin/test-dde-video-preview-plugin.pro
    }
}
