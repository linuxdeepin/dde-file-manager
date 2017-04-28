TARGET = controller
TEMPLATE = lib
CONFIG += shared c++11 x86_64
QT += network sql xml
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += qml
DEFINES += TF_DLL

INCLUDEPATH += ../helpers ../models
DEPENDPATH  += ../helpers ../models

include(../appbase.pri)

LIBS += -L$$DESTDIR -lhelper -lmodel


HEADERS += applicationcontroller.h
SOURCES += applicationcontroller.cpp
HEADERS += sharecontroller.h
SOURCES += sharecontroller.cpp

INSTALLS += target
