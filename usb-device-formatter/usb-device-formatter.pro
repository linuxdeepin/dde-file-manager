#-------------------------------------------------
#
# Project created by QtCreator 2016-12-07T09:33:51
#
#-------------------------------------------------

QT       += core gui concurrent network x11extras dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = usb-device-formatter
TEMPLATE = app

PKGCONFIG += x11 udisks2-qt5 dtkwidget dtkgui

CONFIG += c++11 link_pkgconfig

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

SOURCES += main.cpp \
    view/mainwindow.cpp \
    view/mainpage.cpp \
    view/warnpage.cpp \
    view/formatingpage.cpp \
    view/finishpage.cpp \
    view/errorpage.cpp \
    app/cmdmanager.cpp \
    dialogs/messagedialog.cpp \
    app/singletonapp.cpp \
    utils/udisksutils.cpp \
    utils/fsutils.cpp

HEADERS  += \
    view/mainwindow.h \
    view/mainpage.h \
    view/warnpage.h \
    view/formatingpage.h \
    view/finishpage.h \
    view/errorpage.h \
    app/cmdmanager.h \
    dialogs/messagedialog.h \
    app/singletonapp.h \
    utils/udisksutils.h \
    utils/fsutils.h

CONFIG(release, debug|release) {
    # Automating generation .qm files from .ts files
    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
    !system($$PWD/translate_ts2desktop.sh): error("Failed to generate translation")
}

PREFIX = /usr
BINDIR = $$PREFIX/bin
SHAREDIR = $$PREFIX/share/$${TARGET}

target.path = $$BINDIR

translations.files = $$PWD/translations/*.qm
translations.path = $$SHAREDIR/translations

INSTALLS += target translations
