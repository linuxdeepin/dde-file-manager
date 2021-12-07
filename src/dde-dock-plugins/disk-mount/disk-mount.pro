PREFIX = /usr
QT              += core widgets concurrent dbus
TEMPLATE         = lib
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkwidget gio-qt udisks2-qt5

INCLUDEPATH += /usr/include/dde-dock
INCLUDEPATH += $$PWD/../../dde-file-manager-lib/interfaces \
               $$PWD/../../dde-file-manager-lib

TARGET          = $$qtLibraryTarget(dde-disk-mount-plugin)
DESTDIR          = $$_PRO_FILE_PWD_/../
DISTFILES       += disk-mount.json

DEFINES += QT_MESSAGELOGCONTEXT DFM_NO_FILE_WATCHER

#unix: LIBS += -L$$OUT_PWD/../../dde-file-manager-lib -ldde-file-manager

include(src.pri)

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts
TR_EXCLUDE += $$PWD/../../dde-file-manager-lib/configure/*

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/../../dde-file-manager-lib/generate_translations.sh): error("Failed to generate translation")
#    DEFINES += QT_NO_DEBUG_OUTPUT
}

#安全加固
QMAKE_CXXFLAGS += -fstack-protector-all
QMAKE_LFLAGS += -z now -fPIC
isEqual(ARCH, mips64) | isEqual(ARCH, mips32){
    QMAKE_LFLAGS += -z noexecstack -z relro
}

gschema.path = $${PREFIX}/share/glib-2.0/schemas
gschema.files = *.gschema.xml

translations.path = $${PREFIX}/share/$${TARGET}/translations
translations.files = translations/*.qm

target.path = $${PREFIX}/lib/dde-dock/plugins/system-trays/
INSTALLS += target gschema translations

RESOURCES += \
    resources.qrc \
    resources/theme-icons.qrc \
    $$PWD/../../dde-file-manager-lib/configure.qrc
