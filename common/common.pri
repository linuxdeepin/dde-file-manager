unix {
    ProjectName = dde-file-manager
    PREFIX = /usr
    APPSHAREDIR = $$PREFIX/share/$$ProjectName

    CONFIG(release, debug|release) {
        LIB_INSTALL_DIR = $$[QT_INSTALL_LIBS]
    }

    isEmpty(LIB_INSTALL_DIR) {
        PLUGINDIR = ../dde-file-manager-plugins
    } else {
        PLUGINDIR = $$LIB_INSTALL_DIR/$$ProjectName/plugins
    }
    DEFINES += PLUGINDIR=\\\"$$PLUGINDIR\\\"
}
