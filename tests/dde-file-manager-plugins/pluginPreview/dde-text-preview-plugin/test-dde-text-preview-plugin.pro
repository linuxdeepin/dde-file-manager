#-------------------------------------------------
#
# Project created by QtCreator 2017-04-17T11:02:31
#
#-------------------------------------------------

QT       += core gui widgets quick

TARGET = test-dde-text-preview-plugin
TEMPLATE = app
CONFIG += c++11 console

include(../../../../src/common/common.pri)

LIBS += -L$$OUT_PWD/../../../../src/dde-file-manager-lib -ldde-file-manager

DISTFILES += \
    dde-text-preview-plugin.json
#DESTDIR += $$PWD/
#OBJECTS_DIR = $$PWD/debug/obj
#MOC_DIR = $$PWD/debug/moc

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

SOURCES += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-text-preview-plugin/textpreview.cpp \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-text-preview-plugin/textpreviewplugin.cpp

HEADERS += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-text-preview-plugin/textpreview.h \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-text-preview-plugin/textpreviewplugin.h

#include(../../../../3rdparty/googletest/gtest_dependency.pri)
include(../../../../3rdparty/cpp-stub/stub.pri)
include(tests/test.pri)

unix {
    LIBS += -lgtest -lgmock
}
