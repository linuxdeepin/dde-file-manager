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
    }

    isEmpty(LIB_INSTALL_DIR) {
        CONFIG(debug, debug|release) {
            PLUGINDIR = ../dde-file-manager-plugins
        }
    } else {
        PLUGINDIR = $$LIB_INSTALL_DIR/$$ProjectName/plugins
    }

    DEFINES += PLUGINDIR=\\\"$$PLUGINDIR\\\"
}
