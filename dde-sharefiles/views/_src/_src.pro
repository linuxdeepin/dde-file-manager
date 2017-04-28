TARGET = view
TEMPLATE = lib
CONFIG += shared c++11 x86_64
QT += network xml qml
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += qml
DEFINES += TF_DLL

INCLUDEPATH += ../../helpers ../../models
DEPENDPATH  += ../../helpers ../../models

include(../../appbase.pri)

LIBS += -L$$DESTDIR/../../lib -lhelper -lmodel

QMAKE_CLEAN = *.cpp

tmake.target = source.list
tmake.commands = tmake -f ../../config/application.ini -v .. -d . -P
lessThan(QT_MAJOR_VERSION, 5) {
  tmake.depends = qmake
} else {
  tmake.depends = qmake_all
}
QMAKE_EXTRA_TARGETS = tmake
#POST_TARGETDEPS = source.list

include(../../appbase.pri)
#!exists(source.list) {
#  system( $$tmake.commands )
#}
##include(source.list)
