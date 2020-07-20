PREFIX = /usr
QT              += core widgets concurrent dbus
CONFIG          += plugin c++11 link_pkgconfig
PKGCONFIG       += dtkwidget gio-qt udisks2-qt5

INCLUDEPATH += /usr/include/dde-dock
INCLUDEPATH += $$PWD/../../dde-file-manager-lib/interfaces \
               $$PWD/../../dde-file-manager-lib


TARGET = test-dde-disk-mount-plugin
TEMPLATE = app

DESTDIR          = $$_PRO_FILE_PWD_/../
DISTFILES       += disk-mount.json

DEFINES += QT_MESSAGELOGCONTEXT DFM_NO_FILE_WATCHER

#unix: LIBS += -L$$OUT_PWD/../../dde-file-manager-lib -ldde-file-manager

include(src.pri)

TR_EXCLUDE += $$PWD/../../dde-file-manager-lib/configure/*

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/../../dde-file-manager-lib/generate_translations.sh): error("Failed to generate translation")
#    DEFINES += QT_NO_DEBUG_OUTPUT
}

gschema.path = $${PREFIX}/share/glib-2.0/schemas
gschema.files = *.gschema.xml


INSTALLS += target gschema translations

RESOURCES += \
    resources.qrc \
    resources/theme-icons.qrc \
    $$PWD/../../dde-file-manager-lib/configure.qrc

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

include(../../third-party/googletest/gtest_dependency.pri)
include(tests/test.pri)
