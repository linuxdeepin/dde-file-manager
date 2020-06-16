unix {
    ProjectName = dde-file-manager
    PREFIX = /usr
    APPSHAREDIR = $$PREFIX/share/$$ProjectName
    top_srcdir = $$PWD/../

    CONFIG(release, debug|release) {
        LIB_INSTALL_DIR = $$[QT_INSTALL_LIBS]
    }

    ARCH = $$QMAKE_HOST.arch
    isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
        DEFINES += SPLICE_CP
    }

    isEqual(ARCH, x86_64) | isEqual(ARCH, i686) {
        message("Build arch:" $$ARCH)
    } else {
        message("Build arch:" $$ARCH "Deepin Anything support disabled")
        CONFIG += DISABLE_ANYTHING
    }

    isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
        DEFINES += ARCH_MIPSEL ARCH_SW

        #use classical file section mode
        DEFINES += CLASSICAL_SECTION

        DEFINES += LOAD_FILE_INTERVAL=150

        DEFINES += DISABLE_COMPRESS_PREIVEW

        DEFINES += ENABLE_DAEMON
    } else {
        isEmpty(DISABLE_JEMALLOC) {
            LIBS += -ljemalloc
        }
    }

    isEqual(ARCH, sw_64){
        DEFINES += SW_CPUINFO
        QMAKE_CXXFLAGS += -mieee
    }

    isEmpty(LIB_INSTALL_DIR) {
        LIB_BASE_DIR = $$[QT_INSTALL_LIBS]/$$ProjectName
    } else {
        LIB_BASE_DIR = $$LIB_INSTALL_DIR/$$ProjectName
    }

    PLUGINDIR = $$LIB_BASE_DIR/plugins
    TOOLDIR = $$LIB_BASE_DIR/tools

    DEFINES += PLUGINDIR=\\\"$$top_srcdir/plugins:$$PLUGINDIR\\\" TOOLDIR=\\\"$$top_srcdir/tools:$$TOOLDIR\\\"

    isEmpty(VERSION) {
        VERSION = 1.8.2
    }
}

INCLUDEPATH += $$top_srcdir \
               $$top_srcdir/utils \
               $$top_srcdir/dde-file-manager-lib/interfaces \
               $$top_srcdir/dde-file-manager-lib/interfaces/plugins \
               $$top_srcdir/dde-file-manager-plugins/plugininterfaces
