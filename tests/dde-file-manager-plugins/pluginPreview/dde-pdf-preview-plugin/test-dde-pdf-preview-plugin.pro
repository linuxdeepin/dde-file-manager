#-------------------------------------------------
#
# Project created by QtCreator 2017-04-12T09:08:50
#
#-------------------------------------------------

QT       += core gui widgets concurrent quick printsupport

TARGET = test-dde-pdf-preview-plugin
TEMPLATE = app

PKGCONFIG += poppler-cpp

CONFIG += c++11 console link_pkgconfig

include(../../../../src/common/common.pri)

LIBS += -L$$OUT_PWD/../../../../src/dde-file-manager-lib -ldde-file-manager

DISTFILES += dde-pdf-preview-plugin.json

QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0 -DGTEST
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0 -DGTEST

SOURCES += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/pdfwidget.cpp \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/pdfpreview.cpp \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/pdfpreviewplugin.cpp

HEADERS += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/pdfwidget.h \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/pdfpreview.h \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/pdfwidget_p.h \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/pdfpreviewplugin.h

RESOURCES += \
    $$PWD/../../../../src/dde-file-manager-plugins/pluginPreview/dde-pdf-preview-plugin/theme.qrc

#include(../../../../3rdparty/googletest/gtest_dependency.pri)
include(../../../../3rdparty/cpp-stub/stub.pri)
include(tests/test.pri)

unix {
    LIBS += -lgtest -lgmock
}
