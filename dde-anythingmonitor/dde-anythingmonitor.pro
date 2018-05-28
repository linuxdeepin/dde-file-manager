

QT += core \
      dbus \
      concurrent

CONFIG += c++11 \
          console \
          link_pkgconfig


TARGET = dde-anythingmonitor

CONFIG -= app_bundle \
          create_pc \
          create_prl \
          no_install_prl


TEMPLATE = lib

CONFIG(debug, debug|release){
    DEPENDPATH += $$PWD/../dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../dde-file-manager-lib
}




LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager

PKGCONFIG += deepin-anything-server-lib


INCLUDEPATH += $$PWD/../dde-file-manager-lib \
               $$PWD/../dde-file-manager-lib/interfaces \
               $$PWD/../dde-file-manager-lib/shutil



SOURCES += main.cpp \
    taghandle.cpp


unix{

target.path = $$system($$pkgConfigExecutable() --variable libdir deepin-anything-server-lib)/deepin-anything-server-lib/plugins/handlers
INSTALLS += target

}



# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    taghandle.h

DISTFILES += \
    taghandle_as_plugin.json
