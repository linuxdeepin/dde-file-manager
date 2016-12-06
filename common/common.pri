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
