

QT -= gui

QT += core \
      dbus \
      concurrent

CONFIG += c++11 \
          console

TARGET = dde-anythingmonitor
CONFIG -= app_bundle


TEMPLATE = app

LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

CONFIG(debug, debug|release){
    DEPENDPATH += $$PWD/../dde-file-manager-lib
}


INCLUDEPATH += $$PWD/../dde-file-manager-lib \
               $$PWD/../dde-file-manager-lib/interfaces \
               $$PWD/../dde-file-manager-lib/shutil





SOURCES += main.cpp


target.path = /usr/bin

service.path = /lib/systemd/system
service.files = service/dde-anythingmonitor.service


INSTALLS += target service



# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
