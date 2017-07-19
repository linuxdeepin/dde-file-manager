#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

include(../common/common.pri)

QT       += core gui svg dbus x11extras concurrent multimedia
#private
QT       += gui-private

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = $$ProjectName

TEMPLATE = lib
CONFIG += create_pc create_prl no_install_prl


DEFINES += QMAKE_TARGET=\\\"$$TARGET\\\" QMAKE_VERSION=\\\"$$VERSION\\\"

isEmpty(QMAKE_ORGANIZATION_NAME) {
    DEFINES += QMAKE_ORGANIZATION_NAME=\\\"deepin\\\"
}

isEmpty(PREFIX){
    PREFIX = /usr
}

PKGCONFIG += gtk+-2.0 gsettings-qt libsecret-1 gio-unix-2.0 poppler-cpp
CONFIG(release, release|debug) {
    PKGCONFIG += dtkbase dtkwidget dtkutil
} else {
    PKGCONFIG += dtkbase dtkwidget dtkutil
}
PKGCONFIG += dtksettings dtksettingsview
CONFIG += c++11 link_pkgconfig
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

LIBS += -lmagic -lffmpegthumbnailer

include(../dialogs/dialogs.pri)
include(../utils/utils.pri)
include(../chinese2pinyin/chinese2pinyin.pri)
include(../simpleini/simpleini.pri)
include(../fileoperations/fileoperations.pri)
include(deviceinfo/deviceinfo.pri)
include(dbusinterface/dbusinterface.pri)
include(../usershare/usershare.pri)
include(../dde-file-manager-plugins/plugininterfaces/plugininterfaces.pri)
include(../partman/partman.pri)
isEqual(ARCH, sw_64){
#    isEqual(ENABLE_SW_LABLE, YES){
        DEFINES += SW_LABEL
        include(./sw_label/sw_label.pri)
#    }
}

RESOURCES += \
    skin/skin.qrc \
    skin/dialogs.qrc \
    skin/filemanager.qrc \
    themes/themes.qrc \
    configure.qrc

HEADERS += \
    controllers/appcontroller.h \
    views/dleftsidebar.h \
    views/dtoolbar.h \
    views/dfileview.h \
    views/ddetailview.h \
    views/dicontextbutton.h \
    views/dstatebutton.h \
    views/dcheckablebutton.h \
    controllers/filecontroller.h \
    app/filesignalmanager.h \
    views/fileitem.h \
    views/dsearchbar.h \
    models/desktopfileinfo.h \
    models/bookmark.h \
    models/searchhistory.h \
    models/fmsetting.h \
    models/fmstate.h \
    controllers/bookmarkmanager.h \
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
    views/historystack.h\
    dialogs/propertydialog.h \
    views/dhoverbutton.h \
    views/dbookmarkscene.h \
    views/dbookmarkitem.h \
    views/dbookmarkitemgroup.h \
    views/dbookmarkrootitem.h \
    views/dbookmarkview.h \
    controllers/trashmanager.h \
    views/dsplitter.h \
    models/trashfileinfo.h \
    shutil/mimesappsmanager.h \
    views/dbookmarkline.h \
    views/dsplitterhandle.h \
    dialogs/openwithdialog.h \
    controllers/searchcontroller.h \
    models/searchfileinfo.h\
    dialogs/basedialog.h \
    views/extendview.h \
    controllers/pathmanager.h \
    views/ddragwidget.h \
    shutil/mimetypedisplaymanager.h \
    views/dstatusbar.h \
    controllers/subscriber.h \
    models/dfileselectionmodel.h \
    dialogs/closealldialogindicator.h \
    gvfs/gvfsmountclient.h \
    gvfs/mountaskpassworddialog.h \
    gvfs/networkmanager.h \
    gvfs/secretmanager.h \
    models/networkfileinfo.h \
    controllers/networkcontroller.h \
    dialogs/openwithotherdialog.h \
    dialogs/trashpropertydialog.h \
    views/dbookmarkmountedindicatoritem.h \
    controllers/jobcontroller.h \
    shutil/filessizeworker.h \
    views/computerview.h \
    views/flowlayout.h \
    shutil/shortcut.h \
    views/dtabbar.h \
    views/dfiledialog.h \
    interfaces/dfiledialoghandle.h \
    dialogs/shareinfoframe.h \
    interfaces/dfmstandardpaths.h \
    interfaces/dfmglobal.h \
    controllers/sharecontroler.h \
    models/sharefileinfo.h \
    interfaces/dfileviewhelper.h \
    interfaces/diconitemdelegate.h \
    views/fileviewhelper.h \
    interfaces/dlistitemdelegate.h \
    interfaces/dstyleditemdelegate.h \
    interfaces/durl.h \
    interfaces/dfilemenu.h \
    interfaces/ddiriterator.h \
    interfaces/private/dstyleditemdelegate_p.h \
    interfaces/dfilesystemmodel.h \
    app/define.h \
    interfaces/dabstractfilecontroller.h \
    interfaces/dabstractfileinfo.h \
    interfaces/dfilemenumanager.h \
    interfaces/dfileservices.h \
    interfaces/dfmevent.h \
    dialogs/usersharepasswordsettingdialog.h\
    interfaces/private/dabstractfileinfo_p.h \
    interfaces/dfileinfo.h \
    interfaces/private/dfileinfo_p.h \
    interfaces/dfilesystemwatcher.h \
    interfaces/private/dfilesystemwatcher_p.h \
    interfaces/dabstractfilewatcher.h \
    interfaces/dfilewatcher.h \
    interfaces/private/dabstractfilewatcher_p.h \
    interfaces/dfileproxywatcher.h \
    plugins/pluginmanager.h \
    plugins/plugininterfaces/menu/menuinterface.h \
    interfaces/dthumbnailprovider.h \
    views/progressline.h \
    controllers/avfsfilecontroller.h \
    models/avfsfileinfo.h \
    interfaces/dfileiconprovider.h \
    interfaces/dfilewatchermanager.h \
    dialogs/computerpropertydialog.h \
    interfaces/dmimedatabase.h \
    gvfs/qdrive.h \
    gvfs/qvolume.h \
    gvfs/qmount.h \
    gvfs/gvfsmountmanager.h \
    gvfs/qdiskinfo.h \
    interfaces/dfmsetting.h \
    shutil/viewstatesmanager.h \
    interfaces/dfmeventdispatcher.h \
    interfaces/dfmabstracteventhandler.h \
    controllers/fileeventprocessor.h \
    dialogs/previewdialog.h \
    interfaces/dfmbaseview.h \
    interfaces/dfmviewmanager.h \
    views/dfmactionbutton.h \
    interfaces/plugins/dfmviewplugin.h \
    interfaces/plugins/dfmviewfactory.h \
    interfaces/plugins/dfmfactoryloader.h \
    interfaces/plugins/dfmfilecontrollerplugin.h \
    interfaces/plugins/dfmfilecontrollerfactory.h \
    interfaces/plugins/dfmgenericplugin.h \
    interfaces/plugins/dfmgenericfactory.h \
    views/filedialogstatusbar.h \
    interfaces/plugins/dfmfilepreviewplugin.h \
    interfaces/plugins/dfmfilepreviewfactory.h \
    interfaces/dfmfilepreview.h \
    dialogs/filepreviewdialog.h
SOURCES += \
    controllers/appcontroller.cpp \
    views/dleftsidebar.cpp \
    views/dtoolbar.cpp \
    views/dfileview.cpp \
    views/ddetailview.cpp \
    views/dicontextbutton.cpp \
    views/dstatebutton.cpp \
    views/dcheckablebutton.cpp \
    controllers/filecontroller.cpp \
    views/fileitem.cpp \
    views/dsearchbar.cpp \
    models/desktopfileinfo.cpp \
    models/bookmark.cpp \
    models/searchhistory.cpp \
    models/fmsetting.cpp \
    models/fmstate.cpp \
    controllers/bookmarkmanager.cpp \
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
    views/historystack.cpp\
    dialogs/propertydialog.cpp \
    views/dhoverbutton.cpp \
    views/dbookmarkscene.cpp \
    views/dbookmarkitem.cpp \
    views/dbookmarkitemgroup.cpp \
    views/dbookmarkrootitem.cpp \
    views/dbookmarkview.cpp \
    controllers/trashmanager.cpp \
    views/dsplitter.cpp \
    models/trashfileinfo.cpp \
    shutil/mimesappsmanager.cpp \
    views/dbookmarkline.cpp \
    views/dsplitterhandle.cpp \
    dialogs/openwithdialog.cpp \
    controllers/searchcontroller.cpp \
    models/searchfileinfo.cpp\
    dialogs/basedialog.cpp \
    views/extendview.cpp \
    controllers/pathmanager.cpp \
    views/ddragwidget.cpp \
    shutil/mimetypedisplaymanager.cpp \
    views/dstatusbar.cpp \
    controllers/subscriber.cpp \
    models/dfileselectionmodel.cpp \
    dialogs/closealldialogindicator.cpp \
    gvfs/gvfsmountclient.cpp \
    gvfs/mountaskpassworddialog.cpp \
    gvfs/networkmanager.cpp \
    gvfs/secretmanager.cpp \
    models/networkfileinfo.cpp \
    controllers/networkcontroller.cpp \
    dialogs/openwithotherdialog.cpp \
    dialogs/trashpropertydialog.cpp \
    views/dbookmarkmountedindicatoritem.cpp \
    controllers/jobcontroller.cpp \
    shutil/filessizeworker.cpp \
    views/computerview.cpp \
    views/flowlayout.cpp \
    shutil/shortcut.cpp \
    views/dtabbar.cpp \
    views/dfiledialog.cpp \
    interfaces/dfiledialoghandle.cpp \
    dialogs/shareinfoframe.cpp \
    interfaces/dfmstandardpaths.cpp \
    interfaces/dfmglobal.cpp \
    controllers/sharecontroler.cpp \
    models/sharefileinfo.cpp \
    interfaces/dfileviewhelper.cpp \
    interfaces/diconitemdelegate.cpp \
    views/fileviewhelper.cpp \
    interfaces/dlistitemdelegate.cpp \
    interfaces/dstyleditemdelegate.cpp \
    interfaces/durl.cpp \
    interfaces/dfilemenu.cpp \
    interfaces/dfilesystemmodel.cpp \
    interfaces/dabstractfilecontroller.cpp \
    interfaces/dabstractfileinfo.cpp \
    interfaces/dfilemenumanager.cpp \
    interfaces/dfileservices.cpp \
    interfaces/dfmevent.cpp \
    dialogs/usersharepasswordsettingdialog.cpp\
    interfaces/dfileinfo.cpp \
    interfaces/dfilesystemwatcher.cpp \
    interfaces/dabstractfilewatcher.cpp \
    interfaces/dfilewatcher.cpp \
    interfaces/dfileproxywatcher.cpp \
    app/filesignalmanager.cpp \
    plugins/pluginmanager.cpp \
    interfaces/dthumbnailprovider.cpp \
    views/progressline.cpp \
    controllers/avfsfilecontroller.cpp \
    models/avfsfileinfo.cpp \
    interfaces/dfileiconprovider.cpp \
    interfaces/dfilewatchermanager.cpp \
    dialogs/computerpropertydialog.cpp \
    interfaces/dmimedatabase.cpp \
    gvfs/qdrive.cpp \
    gvfs/qvolume.cpp \
    gvfs/qmount.cpp \
    gvfs/gvfsmountmanager.cpp \
    gvfs/qdiskinfo.cpp \
    interfaces/dfmsetting.cpp \
    configure/dfmconfigdict.cpp \
    shutil/viewstatesmanager.cpp \
    interfaces/dfmeventdispatcher.cpp \
    interfaces/dfmabstracteventhandler.cpp \
    controllers/fileeventprocessor.cpp \
    dialogs/previewdialog.cpp \
    interfaces/dfmbaseview.cpp \
    interfaces/dfmviewmanager.cpp \
    views/dfmactionbutton.cpp \
    interfaces/plugins/dfmviewplugin.cpp \
    interfaces/plugins/dfmviewfactory.cpp \
    interfaces/plugins/dfmfactoryloader.cpp \
    interfaces/plugins/dfmfilecontrollerplugin.cpp \
    interfaces/plugins/dfmfilecontrollerfactory.cpp \
    interfaces/plugins/dfmgenericplugin.cpp \
    interfaces/plugins/dfmgenericfactory.cpp \
    views/filedialogstatusbar.cpp \
    interfaces/plugins/dfmfilepreviewplugin.cpp \
    interfaces/plugins/dfmfilepreviewfactory.cpp \
    interfaces/dfmfilepreview.cpp \
    dialogs/filepreviewdialog.cpp

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
    target.path = $$[QT_INSTALL_LIBS]
} else {
    target.path = $$LIB_INSTALL_DIR
}

isEmpty(INCLUDE_INSTALL_DIR) {
    includes.path = $$PREFIX/include/dde-file-manager
} else {
    includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager
}

includes.files += $$PWD/interfaces/*.h $$PWD/interfaces/plugins/*.h

includes_private.path = $${includes.path}/private
includes_private.files += $$PWD/interfaces/private/*.h

isEmpty(INCLUDE_INSTALL_DIR) {
    gvfs_includes.path = $$PREFIX/include/dde-file-manager/gvfs
} else {
    gvfs_includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager/gvfs
}

gvfs_includes.files += $$PWD/gvfs/*.h

isEmpty(INCLUDE_INSTALL_DIR) {
    plugin_includes.path = $$PREFIX/include/dde-file-manager/dde-file-manager-plugins
} else {
    plugin_includes.path = $$INCLUDE_INSTALL_DIR/dde-file-manager/dde-file-manager-plugins
}

plugin_includes.files += $$PWD/../dde-file-manager-plugins/plugininterfaces/menu/*.h
plugin_includes.files += $$PWD/../dde-file-manager-plugins/plugininterfaces/preview/*.h
plugin_includes.files += $$PWD/../dde-file-manager-plugins/plugininterfaces/view/*.h

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

mimetypeAssociations.path = $$APPSHAREDIR/mimetypeassociations
mimetypeAssociations.files += \
    mimetypeassociations/mimetypeassociations.json

TRANSLATIONS += $$PWD/translations/$${TARGET}.ts \
    $$PWD/translations/$${TARGET}_zh_CN.ts

# Automating generation .qm files from .ts files
CONFIG(release, debug|release) {
    !system($$PWD/generate_translations.sh): error("Failed to generate translation")
#    DEFINES += QT_NO_DEBUG_OUTPUT
}

translations.path = $$APPSHAREDIR/translations
translations.files = translations/*.qm

help.path = $$HELPSHAREDIR
help.files = help/*

icon.path = $$ICONDIR
icon.files = skin/images/$${TARGET}.svg

defaultConfig.path = $$APPSHAREDIR/config
defaultConfig.files = configure/default-view-states.json

INSTALLS += target templateFiles translations mimetypeFiles mimetypeAssociations help icon includes includes_private gvfs_includes plugin_includes defaultConfig

DISTFILES += \
    mimetypeassociations/mimetypeassociations.json \
    configure/default-view-states.json \
    skin/qss/filemanageradmin.qss \
    themes/light/DFileViewRoot.theme \
    themes/light/PreviewDialogDefault \
    themes/light/MusicPreviewDialog.theme
