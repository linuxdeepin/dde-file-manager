#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

QT       += core gui widgets svg dbus x11extras network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-file-manager
TEMPLATE = app

isEmpty(PREFIX){
    PREFIX = /usr
}

include(./widgets/widgets.pri)
include(./dialogs/dialogs.pri)
include(./utils/utils.pri)
include(./filemonitor/filemonitor.pri)
include(./deviceinfo/deviceinfo.pri)

PKGCONFIG += x11 gtk+-2.0 xcb xcb-ewmh gsettings-qt dtkbase dtkutil dtkwidget libudev
CONFIG += c++11 link_pkgconfig
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

LIBS += -lmagic
# Automating generation .qm files from .ts files
# system($$PWD/desktop/translate_generation.sh)


RESOURCES += \
    skin/skin.qrc \
    skin/dialogs.qrc \
    skin/filemanager.qrc \
    filemanager/themes/themes.qrc

HEADERS += \
    filemanager/app/define.h \
    filemanager/app/global.h \
    filemanager/controllers/appcontroller.h \
    filemanager/app/filemanagerapp.h \
    filemanager/views/dmovablemainwindow.h \
    filemanager/views/dleftsidebar.h \
    filemanager/views/dtoolbar.h \
    filemanager/views/dfileview.h \
    filemanager/views/ddetailview.h \
    filemanager/views/dicontextbutton.h \
    filemanager/views/dstatebutton.h \
    filemanager/views/dcheckablebutton.h \
    filemanager/models/dfilesystemmodel.h \
    filemanager/controllers/filecontroller.h \
    filemanager/app/filesignalmanager.h \
    filemanager/views/fileitem.h \
    filemanager/views/filemenumanager.h \
    filemanager/views/dsearchbar.h \
    filemanager/views/dtabbar.h \
    filemanager/views/dtabbaritem.h \
    filemanager/views/dfileitemdelegate.h \
    filemanager/shutil/desktopparse.h \
    filemanager/shutil/mimeutils.h \
    filemanager/models/fileinfo.h \
    filemanager/models/desktopfileinfo.h \
    filemanager/shutil/iconprovider.h \
    filemanager/models/bookmark.h \
    filemanager/models/imagefileinfo.h \
    filemanager/models/searchhistory.h \
    filemanager/models/fmsetting.h \
    filemanager/models/fmstate.h \
    filemanager/controllers/bookmarkmanager.h \
    filemanager/controllers/recenthistorymanager.h \
    filemanager/controllers/fmsettingmanager.h \
    filemanager/controllers/fmstatemanager.h \
    filemanager/controllers/basemanager.h \
    filemanager/dialogs/dialogmanager.h \
    filemanager/controllers/searchhistroymanager.h \
    filemanager/views/windowmanager.h \
    filemanager/shutil/desktopfile.h \
    filemanager/shutil/fileutils.h \
    filemanager/shutil/properties.h \
    filemanager/views/dfilemanagerwindow.h \
    filemanager/views/dcrumbwidget.h \
    filemanager/views/dcrumbbutton.h \
    filemanager/views/dhorizseparator.h \
    filemanager/models/ddirmodel.h \
    filemanager/views/dscrollbar.h \
    filemanager/app/fmevent.h \
    filemanager/views/historystack.h\
    filemanager/dialogs/propertydialog.h \
    filemanager/controllers/filejob.h \
    filemanager/views/dfilemenu.h \
    filemanager/views/dhoverbutton.h \
    filemanager/views/bmlistwidget.h \
    filemanager/views/bmlistwidgetitem.h \
    filemanager/views/dbookmarkscene.h \
    filemanager/views/dbookmarkitem.h \
    filemanager/views/dbookmarkitemgroup.h \
    filemanager/views/dbookmarkrootitem.h \
    filemanager/views/dbookmarkview.h \
    filemanager/controllers/trashmanager.h \
    filemanager/views/dsplitter.h \
    filemanager/models/abstractfileinfo.h \
    filemanager/controllers/fileservices.h \
    filemanager/controllers/abstractfilecontroller.h \
    filemanager/models/recentfileinfo.h \
    filemanager/app/singleapplication.h \
    filemanager/app/logutil.h \
    filemanager/models/trashfileinfo.h \
    filemanager/shutil/mimesappsmanager.h \
    filemanager/views/dbookmarkline.h \
    filemanager/views/dsplitterhandle.h \
    filemanager/dialogs/openwithdialog.h \
    filemanager/models/durl.h \
    filemanager/controllers/searchcontroller.h \
    filemanager/models/searchfileinfo.h\
    filemanager/shutil/standardpath.h


SOURCES += \
    filemanager/controllers/appcontroller.cpp \
    filemanager/main.cpp \
    filemanager/app/filemanagerapp.cpp \
    filemanager/views/dmovablemainwindow.cpp \
    filemanager/views/dleftsidebar.cpp \
    filemanager/views/dtoolbar.cpp \
    filemanager/views/dfileview.cpp \
    filemanager/views/ddetailview.cpp \
    filemanager/views/dicontextbutton.cpp \
    filemanager/views/dstatebutton.cpp \
    filemanager/views/dcheckablebutton.cpp \
    filemanager/models/dfilesystemmodel.cpp \
    filemanager/controllers/filecontroller.cpp \
    filemanager/views/fileitem.cpp \
    filemanager/views/filemenumanager.cpp \
    filemanager/views/dsearchbar.cpp \
    filemanager/views/dtabbar.cpp \
    filemanager/views/dtabbaritem.cpp \
    filemanager/views/dfileitemdelegate.cpp \
    filemanager/shutil/desktopparse.cpp \
    filemanager/shutil/mimeutils.cpp \
    filemanager/models/fileinfo.cpp \
    filemanager/models/desktopfileinfo.cpp \
    filemanager/shutil/iconprovider.cpp \
    filemanager/models/bookmark.cpp \
    filemanager/models/imagefileinfo.cpp \
    filemanager/models/searchhistory.cpp \
    filemanager/models/fmsetting.cpp \
    filemanager/models/fmstate.cpp \
    filemanager/controllers/bookmarkmanager.cpp \
    filemanager/controllers/recenthistorymanager.cpp \
    filemanager/controllers/fmsettingmanager.cpp \
    filemanager/controllers/fmstatemanager.cpp \
    filemanager/controllers/basemanager.cpp \
    filemanager/dialogs/dialogmanager.cpp \
    filemanager/controllers/searchhistroymanager.cpp \
    filemanager/views/windowmanager.cpp \
    filemanager/shutil/desktopfile.cpp \
    filemanager/shutil/fileutils.cpp \
    filemanager/shutil/properties.cpp \
    filemanager/views/dfilemanagerwindow.cpp \
    filemanager/views/dcrumbwidget.cpp \
    filemanager/views/dcrumbbutton.cpp \
    filemanager/views/dhorizseparator.cpp \
    filemanager/models/ddirmodel.cpp \
    filemanager/views/dscrollbar.cpp \
    filemanager/app/fmevent.cpp \
    filemanager/views/historystack.cpp\
    filemanager/dialogs/propertydialog.cpp \
    filemanager/controllers/filejob.cpp \
    filemanager/views/dfilemenu.cpp \
    filemanager/views/dhoverbutton.cpp \
    filemanager/views/bmlistwidget.cpp \
    filemanager/views/bmlistwidgetitem.cpp \
    filemanager/views/dbookmarkscene.cpp \
    filemanager/views/dbookmarkitem.cpp \
    filemanager/views/dbookmarkitemgroup.cpp \
    filemanager/views/dbookmarkrootitem.cpp \
    filemanager/views/dbookmarkview.cpp \
    filemanager/controllers/trashmanager.cpp \
    filemanager/views/dsplitter.cpp \
    filemanager/models/abstractfileinfo.cpp \
    filemanager/controllers/fileservices.cpp \
    filemanager/controllers/abstractfilecontroller.cpp \
    filemanager/models/recentfileinfo.cpp \
    filemanager/app/singleapplication.cpp \
    filemanager/app/logutil.cpp \
    filemanager/models/trashfileinfo.cpp \
    filemanager/shutil/mimesappsmanager.cpp \
    filemanager/views/dbookmarkline.cpp \
    filemanager/views/dsplitterhandle.cpp \
    filemanager/dialogs/openwithdialog.cpp \
    filemanager/models/durl.cpp \
    filemanager/controllers/searchcontroller.cpp \
    filemanager/models/searchfileinfo.cpp\
    filemanager/shutil/standardpath.cpp



INCLUDEPATH += filemanager/models

target.path = /usr/bin/


desktop.path = $${PREFIX}/share/applications/
desktop.files = dde-file-manager.desktop

INSTALLS += target desktop

