isEmpty(LIB_INSTALL_DIR) {
   TOOLDIR = $$[QT_INSTALL_LIBS]/dde-file-manager/tools/thumbnail
} else {
   TOOLDIR = $$LIB_INSTALL_DIR/dde-file-manager/tools/thumbnail
}

top_dir = $$PWD/..

DESTDIR = $$top_dir/tools/thumbnail

target.path = $$TOOLDIR
json.files += $$_PRO_FILE_PWD_/$${TARGET}.json
json.path = $${target.path}

INSTALLS += target json
OTHER_FILES += $$json.files

CONFIG(debug, debug|release) {
    mkpath($$DESTDIR)
    system(ln -s $$json.files $$DESTDIR)
}
