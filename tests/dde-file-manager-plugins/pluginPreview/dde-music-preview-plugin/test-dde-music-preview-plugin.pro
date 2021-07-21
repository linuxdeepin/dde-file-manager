#-------------------------------------------------
#
# Project created by QtCreator 2017-04-20T15:40:08
#
#-------------------------------------------------

QT       += core gui widgets multimedia quick

TARGET = test-dde-music-preview-plugin
TEMPLATE = app
CONFIG += c++11 console link_pkgconfig

PKGCONFIG += taglib

include(../../../../src/common/common.pri)

LIBS += -L$$OUT_PWD/../../../../src/dde-file-manager-lib -ldde-file-manager -licui18n

DISTFILES += dde-music-preview-plugin.json

#DESTDIR += $$PWD/
#OBJECTS_DIR = $$PWD/debug/obj
#MOC_DIR = $$PWD/debug/moc

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0 -DGTEST
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

SOURCES += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/toolbarframe.cpp \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/musicpreview.cpp \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/musicmessageview.cpp \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/musicpreviewplugin.cpp

HEADERS += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/toolbarframe.h \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/musicmessageview.h \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/musicpreview.h \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/musicpreviewplugin.h

RESOURCES += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-music-preview-plugin/dde-music-preview-plugin.qrc

#include(../../../../3rdparty/googletest/gtest_dependency.pri)
include(../../../../3rdparty/cpp-stub/stub.pri)
include(tests/test.pri)

unix {
    LIBS += -lgtest -lgmock
}
