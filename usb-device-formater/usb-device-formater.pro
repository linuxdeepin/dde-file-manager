#-------------------------------------------------
#
# Project created by QtCreator 2016-12-07T09:33:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = usb-device-formatter
TEMPLATE = app

PKGCONFIG += dtkwidget dtkbase

CONFIG += c++11 link_pkgconfig

SOURCES += main.cpp \
    mainwindow.cpp \
    widgets/progressline.cpp \
    mainpage.cpp \
    warnpage.cpp \
    formatingpage.cpp \
    widgets/progressbox.cpp \
    finishpage.cpp \
    errorpage.cpp

HEADERS  += \
    mainwindow.h \
    widgets/progressline.h \
    mainpage.h \
    warnpage.h \
    formatingpage.h \
    widgets/progressbox.h \
    finishpage.h \
    errorpage.h

RESOURCES += \
    theme/theme.qrc
