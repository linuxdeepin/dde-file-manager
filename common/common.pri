unix {
    ProjectName = dde-file-manager
    PREFIX = /usr
    APPSHAREDIR = $$PREFIX/share/$$ProjectName

    CONFIG(release, debug|release) {
        LIB_INSTALL_DIR = $$[QT_INSTALL_LIBS]
    }

    ARCH = $$QMAKE_HOST.arch
    isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
        DEFINES += MENU_DIALOG_PLUGIN

        DEFINES += SPLICE_CP
    }

    isEqual(ARCH, sw_64) | isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
        DEFINES += ARCH_MIPSEL

        #use classical file section mode
        DEFINES += CLASSICAL_SECTION
        DEFINES += AUTO_RESTART_DEAMON

        DEFINES += LOAD_FILE_INTERVAL=150

        DEFINES += DDE_COMPUTER_TRASH

        DEFINES += DISABLE_COMPRESS_PREIVEW

        DEFINES += DISABLE_QUIT_ON_LAST_WINDOW_CLOSED
    }

    isEqual(ARCH, sw_64){
        DEFINES += SW_CPUINFO
        QMAKE_CXXFLAGS += -mieee
    }

    isEmpty(LIB_INSTALL_DIR) {
        PLUGINDIR = $$[QT_INSTALL_LIBS]/$$ProjectName/plugins
    } else {
        PLUGINDIR = $$LIB_INSTALL_DIR/$$ProjectName/plugins
    }

    DEFINES += PLUGINDIR=\\\"$$top_srcdir/plugins:$$PLUGINDIR\\\"

    isEmpty(VERSION) {
        VERSION = 1.5
    }

}

INCLUDEPATH += $$top_srcdir\
               $$top_srcdir/utils\
               $$top_srcdir/dde-file-manager-lib/interfaces\
               $$top_srcdir/dde-file-manager-lib/interfaces/plugins\
               $$top_srcdir/dde-file-manager-plugins/plugininterfaces
