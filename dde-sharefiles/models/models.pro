TARGET = model
TEMPLATE = lib
CONFIG += shared c++11 x86_64
QT += sql
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += qml
DEFINES += TF_DLL

INCLUDEPATH += ../helpers sqlobjects mongoobjects
DEPENDPATH  += ../helpers sqlobjects mongoobjects

include(../appbase.pri)

LIBS += -L$$DESTDIR -lhelper

INSTALLS += target
