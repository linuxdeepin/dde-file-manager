TEMPLATE = subdirs

include(../common/common.pri)

IS_TEST_VIEW_PLUGIN = False

isEqual(ARCH, mips64) | isEqual(ARCH, mips32) | isEqual(ARCH, loongarch64) {
#    SUBDIRS += \
#        showfile
}

isEqual(IS_TEST_VIEW_PLUGIN, True){
    SUBDIRS += \
        pluginView
}

SUBDIRS += \
    pluginPreview

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

CONFIG(release, debug|release) {
    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
    !system($$PWD/update_translations.sh): error("Failed to generate translation")
}

unix {
    target.path = $$PLUGIN_INSTALL_DIR

    translations.path = $${PREFIX}/share/dde-file-manager/translations
    translations.files = $$PWD/translations/*.qm
    INSTALLS += target translations
}
