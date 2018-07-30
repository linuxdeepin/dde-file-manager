TEMPLATE = subdirs

SUBDIRS += \
    dde-image-preview-plugin \
    dde-pdf-preview-plugin \
    dde-text-preview-plugin \
    dde-music-preview-plugin

!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    SUBDIRS += dde-video-preview-plugin
}
