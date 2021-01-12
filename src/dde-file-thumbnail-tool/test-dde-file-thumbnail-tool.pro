TEMPLATE = subdirs

!CONFIG(DISABLE_FFMPEG):!isEqual(BUILD_MINIMUM, YES) {
    SUBDIRS += video/test-video.pro
}
