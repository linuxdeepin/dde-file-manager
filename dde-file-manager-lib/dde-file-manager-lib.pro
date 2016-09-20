#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
system($$PWD/../vendor/prebuild)
include($$PWD/../vendor/vendor.pri)

QT       += core gui svg dbus x11extras network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-file-manager

TEMPLATE = lib
CONFIG += create_pc create_prl no_install_prl

isEmpty(VERSION) {
    VERSION = 1.0
}

ARCH = $$QMAKE_HOST.arch
isEqual(ARCH, mips64) | isEqual(ARCH, mips32) {
    DEFINES += ARCH_MIPSEL

    #use classical file section mode
    DEFINES += CLASSICAL_SECTION
    DEFINES += AUTO_RESTART_DEAMON
}

isEmpty(PREFIX){
    PREFIX = /usr
}

include(../widgets/widgets.pri)
include(../dialogs/dialogs.pri)
include(../utils/utils.pri)
include(../filemonitor/filemonitor.pri)
include(../chinese2pinyin/chinese2pinyin.pri)
include(../xdnd/xdnd.pri)
include(../simpleini/simpleini.pri)
include(../fileoperations/fileoperations.pri)
include(deviceinfo/deviceinfo.pri)
include(dbusinterface/dbusinterface.pri)
include(../thumbnailer/thumbnailer.pri)
include(../usershare/usershare.pri)

PKGCONFIG += x11 gtk+-2.0 xcb xcb-ewmh gsettings-qt libudev x11 xext libsecret-1\
             gio-unix-2.0 poppler-qt5 libstartup-notification-1.0 xcb-aux
CONFIG += c++11 link_pkgconfig
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

LIBS += -lmagic -lffmpegthumbnailer

RESOURCES += \
    skin/skin.qrc \
    skin/dialogs.qrc \
    skin/filemanager.qrc \
    themes/themes.qrc

HEADERS += \
    app/define.h \
    app/global.h \
    controllers/appcontroller.h \
    app/filemanagerapp.h \
    views/dmovablemainwindow.h \
    views/dleftsidebar.h \
    views/dtoolbar.h \
    views/dfileview.h \
    views/ddetailview.h \
    views/dicontextbutton.h \
    views/dstatebutton.h \
    views/dcheckablebutton.h \
    models/dfilesystemmodel.h \
    controllers/filecontroller.h \
    app/filesignalmanager.h \
    views/fileitem.h \
    views/filemenumanager.h \
    views/dsearchbar.h \
    views/dfileitemdelegate.h \
    models/fileinfo.h \
    models/desktopfileinfo.h \
    shutil/iconprovider.h \
    models/bookmark.h \
    models/imagefileinfo.h \
    models/searchhistory.h \
    models/fmsetting.h \
    models/fmstate.h \
    controllers/bookmarkmanager.h \
    controllers/recenthistorymanager.h \
    controllers/fmstatemanager.h \
    controllers/basemanager.h \
    dialogs/dialogmanager.h \
    controllers/searchhistroymanager.h \
    views/windowmanager.h \
    shutil/desktopfile.h \
    shutil/fileutils.h \
    shutil/properties.h \
    views/dfilemanagerwindow.h \
    views/dcrumbwidget.h \
    views/dcrumbbutton.h \
    views/dhorizseparator.h \
    app/fmevent.h \
    views/historystack.h\
    dialogs/propertydialog.h \
    views/dfilemenu.h \
    views/dhoverbutton.h \
    views/dbookmarkscene.h \
    views/dbookmarkitem.h \
    views/dbookmarkitemgroup.h \
    views/dbookmarkrootitem.h \
    views/dbookmarkview.h \
    controllers/trashmanager.h \
    views/dsplitter.h \
    models/abstractfileinfo.h \
    controllers/fileservices.h \
    controllers/abstractfilecontroller.h \
    models/recentfileinfo.h \
    app/singleapplication.h \
    app/logutil.h \
    models/trashfileinfo.h \
    shutil/mimesappsmanager.h \
    views/dbookmarkline.h \
    views/dsplitterhandle.h \
    dialogs/openwithdialog.h \
    controllers/searchcontroller.h \
    models/searchfileinfo.h\
    shutil/standardpath.h \
    dialogs/basedialog.h \
    models/ddiriterator.h \
    views/extendview.h \
    controllers/pathmanager.h \
    views/ddragwidget.h \
    shutil/mimetypedisplaymanager.h \
    views/dstatusbar.h \
    controllers/subscriber.h \
    models/menuactiontype.h \
    models/dfileselectionmodel.h \
    dialogs/closealldialogindicator.h \
    gvfs/gvfsmountclient.h \
    gvfs/mountaskpassworddialog.h \
    gvfs/networkmanager.h \
    gvfs/secrectmanager.h \
    models/networkfileinfo.h \
    controllers/networkcontroller.h \
    dialogs/openwithotherdialog.h \
    dialogs/trashpropertydialog.h \
    views/dbookmarkmountedindicatoritem.h \
    views/deditorwidgetmenu.h \
    controllers/jobcontroller.h \
    shutil/filessizeworker.h \
    views/computerview.h \
    views/flowlayout.h \
    shutil/shortcut.h \
    views/dtabbar.h \
    views/dfiledialog.h \
    interfaces/dfiledialoghandle.h

SOURCES += \
    controllers/appcontroller.cpp \
    app/filemanagerapp.cpp \
    views/dmovablemainwindow.cpp \
    views/dleftsidebar.cpp \
    views/dtoolbar.cpp \
    views/dfileview.cpp \
    views/ddetailview.cpp \
    views/dicontextbutton.cpp \
    views/dstatebutton.cpp \
    views/dcheckablebutton.cpp \
    models/dfilesystemmodel.cpp \
    controllers/filecontroller.cpp \
    views/fileitem.cpp \
    views/filemenumanager.cpp \
    views/dsearchbar.cpp \
    views/dfileitemdelegate.cpp \
    models/fileinfo.cpp \
    models/desktopfileinfo.cpp \
    shutil/iconprovider.cpp \
    models/bookmark.cpp \
    models/imagefileinfo.cpp \
    models/searchhistory.cpp \
    models/fmsetting.cpp \
    models/fmstate.cpp \
    controllers/bookmarkmanager.cpp \
    controllers/recenthistorymanager.cpp \
    controllers/fmstatemanager.cpp \
    controllers/basemanager.cpp \
    dialogs/dialogmanager.cpp \
    controllers/searchhistroymanager.cpp \
    views/windowmanager.cpp \
    shutil/desktopfile.cpp \
    shutil/fileutils.cpp \
    shutil/properties.cpp \
    views/dfilemanagerwindow.cpp \
    views/dcrumbwidget.cpp \
    views/dcrumbbutton.cpp \
    views/dhorizseparator.cpp \
    app/fmevent.cpp \
    views/historystack.cpp\
    dialogs/propertydialog.cpp \
    views/dfilemenu.cpp \
    views/dhoverbutton.cpp \
    views/dbookmarkscene.cpp \
    views/dbookmarkitem.cpp \
    views/dbookmarkitemgroup.cpp \
    views/dbookmarkrootitem.cpp \
    views/dbookmarkview.cpp \
    controllers/trashmanager.cpp \
    views/dsplitter.cpp \
    models/abstractfileinfo.cpp \
    controllers/fileservices.cpp \
    controllers/abstractfilecontroller.cpp \
    models/recentfileinfo.cpp \
    app/singleapplication.cpp \
    app/logutil.cpp \
    models/trashfileinfo.cpp \
    shutil/mimesappsmanager.cpp \
    views/dbookmarkline.cpp \
    views/dsplitterhandle.cpp \
    dialogs/openwithdialog.cpp \
    controllers/searchcontroller.cpp \
    models/searchfileinfo.cpp\
    shutil/standardpath.cpp \
    dialogs/basedialog.cpp \
    views/extendview.cpp \
    controllers/pathmanager.cpp \
    views/ddragwidget.cpp \
    shutil/mimetypedisplaymanager.cpp \
    views/dstatusbar.cpp \
    controllers/subscriber.cpp \
    models/menuactiontype.cpp \
    models/dfileselectionmodel.cpp \
    dialogs/closealldialogindicator.cpp \
    app/global.cpp \
    gvfs/gvfsmountclient.cpp \
    gvfs/mountaskpassworddialog.cpp \
    gvfs/networkmanager.cpp \
    gvfs/secrectmanager.cpp \
    models/networkfileinfo.cpp \
    controllers/networkcontroller.cpp \
    dialogs/openwithotherdialog.cpp \
    dialogs/trashpropertydialog.cpp \
    views/dbookmarkmountedindicatoritem.cpp \
    views/deditorwidgetmenu.cpp \
    controllers/jobcontroller.cpp \
    shutil/filessizeworker.cpp \
    views/computerview.cpp \
    views/flowlayout.cpp \
    shutil/shortcut.cpp \
    views/dtabbar.cpp \
    views/dfiledialog.cpp \
    interfaces/dfiledialoghandle.cpp

INCLUDEPATH += $$PWD/../ $$PWD/../utils/

APPSHAREDIR = $$PREFIX/share/$$TARGET
HELPSHAREDIR = $$PREFIX/share/dman/$$TARGET
ICONDIR = $$PREFIX/share/icons/hicolor/scalable/apps
DEFINES += APPSHAREDIR=\\\"$$APPSHAREDIR\\\"

win32* {
    DEFINES += STATIC_LIB
    CONFIG += staticlib
    LIB_DIR =
}

isEmpty(LIB_INSTALL_DIR) {
    target.path = $$PREFIX/lib
} else {
    target.path = $$LIB_INSTALL_DIR
}

isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/dde-file-manager-$$VERSION
} else {
    includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager-$$VERSION
}

includes.files += $$PWD/interfaces/*.h

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_NAME = dde-file-manager
QMAKE_PKGCONFIG_DESCRIPTION = DDE File Manager Header Files
QMAKE_PKGCONFIG_INCDIR = $$includes.path

templateFiles.path = $$APPSHAREDIR/templates
templateFiles.files = skin/templates/newDoc.doc \
    skin/templates/newExcel.xls \
    skin/templates/newPowerPoint.ppt \
    skin/templates/newTxt.txt

mimetypeFiles.path = $$APPSHAREDIR/mimetypes
mimetypeFiles.files += \
    mimetypes/archive.mimetype \
    mimetypes/text.mimetype \
    mimetypes/video.mimetype \
    mimetypes/audio.mimetype \
    mimetypes/image.mimetype \
    mimetypes/executable.mimetype

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    system($$PWD/generate_translations.sh)
}

translations.path = $$APPSHAREDIR/translations
translations.files = translations/*.qm

help.path = $$HELPSHAREDIR
help.files = help/*

icon.path = $$ICONDIR
icon.files = skin/images/$${TARGET}.svg

INSTALLS += target templateFiles translations mimetypeFiles help icon includes
