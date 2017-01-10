#-------------------------------------------------
#
# Project created by QtCreator 2016-12-07T09:33:51
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = usb-device-formatter
TEMPLATE = app

include(../partman/partman.pri)

PKGCONFIG += dtkwidget dtkbase dtkutil

CONFIG += c++11 link_pkgconfig

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

SOURCES += main.cpp \
    view/mainwindow.cpp \
    widgets/progressline.cpp \
    view/mainpage.cpp \
    view/warnpage.cpp \
    view/formatingpage.cpp \
    widgets/progressbox.cpp \
    view/finishpage.cpp \
    view/errorpage.cpp \
    app/cmdmanager.cpp \
    dialogs/messagedialog.cpp

HEADERS  += \
    view/mainwindow.h \
    widgets/progressline.h \
    view/mainpage.h \
    view/warnpage.h \
    view/formatingpage.h \
    widgets/progressbox.h \
    view/finishpage.h \
    view/errorpage.h \
    app/cmdmanager.h \
    dialogs/messagedialog.h

RESOURCES += \
    theme/theme.qrc

PREFIX = /usr
BINDIR = $$PREFIX/bin
SHAREDIR = $$PREFIX/share/$${TARGET}

target.path = $$BINDIR

translations.files = $$PWD/translations/*.qm
translations.path = $$SHAREDIR/translations

policy.path = $${PREFIX}/share/polkit-1/actions/
policy.files = pkexec/com.deepin.pkexec.usb-device-formatter.policy

pkexec.files = pkexec/usb-device-formatter-pkexec
pkexec.path = /usr/bin

DISTFILES += \
    pkexec/com.deepin.pkexec.usb-device-formatter.policy \
    pkexec/usb-device-formatter-pkexec

INSTALLS += target translations policy pkexec
