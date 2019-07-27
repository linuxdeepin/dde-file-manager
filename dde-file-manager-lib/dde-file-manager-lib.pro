#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

include(../common/common.pri)

QT       += core gui svg dbus x11extras concurrent multimedia dbus xml KCodecs
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

CONFIG += c++11 link_pkgconfig
PKGCONFIG += gsettings-qt libsecret-1 gio-unix-2.0 poppler-cpp dtkwidget udisks2-qt5 disomaster
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += QT_MESSAGELOGCONTEXT

CONFIG(DISABLE_FFMPEG) | isEqual(BUILD_MINIMUM, YES) {
    DEFINES += DISABLE_FFMEPG
}

# BUILD_MINIMUM for live system
isEqual(BUILD_MINIMUM, YES){
    DEFINES += DFM_MINIMUM
}

CONFIG(DISABLE_ANYTHING) {
    message("Quick search and tag support disabled dut to Anything support disabled.")
    DEFINES += DISABLE_QUICK_SEARCH
    DEFINES += DISABLE_TAG_SUPPORT
}

include(../dialogs/dialogs.pri)
include(../utils/utils.pri)
include(../chinese2pinyin/chinese2pinyin.pri)
include(../fileoperations/fileoperations.pri)
include(deviceinfo/deviceinfo.pri)
include(devicemanagement/devicemanagement.pri)
include(dbusinterface/dbusinterface.pri)
include(../usershare/usershare.pri)
include(../dde-file-manager-plugins/plugininterfaces/plugininterfaces.pri)
include(../partman/partman.pri)
include(tag/tag.pri)

isEqual(ARCH, sw_64){
#    isEqual(ENABLE_SW_LABLE, YES){
        DEFINES += SW_LABEL
        include(./sw_label/sw_label.pri)
#    }
}
include(io/io.pri)
include(interfaces/vfs/vfs.pri)

RESOURCES += \
    skin/skin.qrc \
    skin/dialogs.qrc \
    skin/filemanager.qrc \
    themes/themes.qrc \
    configure.qrc

HEADERS += \
    controllers/appcontroller.h \
    views/dfmsidebaropticaldevitem.h \
    views/dtoolbar.h \
    views/dfileview.h \
    views/ddetailview.h \
    views/dicontextbutton.h \
    views/dstatebutton.h \
    controllers/filecontroller.h \
    app/filesignalmanager.h \
    views/fileitem.h \
    models/desktopfileinfo.h \
    models/bookmark.h \
    models/searchhistory.h \
    controllers/bookmarkmanager.h \
    dialogs/dialogmanager.h \
    controllers/searchhistroymanager.h \
    views/windowmanager.h \
    shutil/desktopfile.h \
    shutil/fileutils.h \
    shutil/properties.h \
    views/dfilemanagerwindow.h \
    views/historystack.h\
    dialogs/propertydialog.h \
    views/dhoverbutton.h \
    views/dfmsidebaritemgroup.h \
    controllers/trashmanager.h \
    models/trashfileinfo.h \
    shutil/mimesappsmanager.h \
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
    gvfs/mountaskpassworddialog.h \
    gvfs/networkmanager.h \
    gvfs/secretmanager.h \
    models/networkfileinfo.h \
    controllers/networkcontroller.h \
    dialogs/trashpropertydialog.h \
    controllers/jobcontroller.h \
    views/computerview.h \
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
    dialogs/filepreviewdialog.h \
    gvfs/mountsecretdiskaskpassworddialog.h \
    views/drenamebar.h \
    shutil/filebatchprocess.h \
    dialogs/ddesktoprenamedialog.h \
    dialogs/dmultifilepropertydialog.h \
    dialogs/movetotrashconflictdialog.h \
    dialogs/dfmsettingdialog.h \
    controllers/operatorrevocation.h \
    views/themeconfig.h \
    controllers/tagcontroller.h \
    models/tagfileinfo.h \
    views/dtagactionwidget.h \
    views/droundbutton.h \
    views/dtagedit.h \
    shutil/dsqlitehandle.h \
    shutil/danythingmonitorfilter.h \
    controllers/tagmanagerdaemoncontroller.h \
    controllers/interface/tagmanagerdaemon_interface.h \
    interfaces/dfmsettings.h \
    interfaces/dfmsidebar.h \
    interfaces/dfmsidebaritem.h \
    views/dfmsidebaritemseparator.h \
    views/dfmsidebardefaultitem.h \
    views/dfmsidebarbookmarkitem.h \
    views/dfmsidebartrashitem.h \
    views/dfmsidebardeviceitem.h \
    views/dfmsidebarnetworkitem.h \
    views/dfmsidebartagitem.h \
    interfaces/dfmcrumbbar.h \
    interfaces/dfmcrumbitem.h \
    interfaces/dfmcrumbinterface.h \
    interfaces/plugins/dfmcrumbplugin.h \
    interfaces/plugins/dfmcrumbfactory.h \
    interfaces/dfmcrumbmanager.h \
    controllers/dfmfilecrumbcontroller.h \
    controllers/dfmcomputercrumbcontroller.h \
    controllers/dfmbookmarkcrumbcontroller.h \
    controllers/dfmtagcrumbcontroller.h \
    controllers/dfmnetworkcrumbcontroller.h \
    controllers/dfmtrashcrumbcontroller.h \
    controllers/dfmusersharecrumbcontroller.h \
    controllers/dfmavfscrumbcontroller.h \
    controllers/dfmsmbcrumbcontroller.h \
    controllers/dfmmtpcrumbcontroller.h \
    views/dfmaddressbar.h \
    views/dcompleterlistview.h \
    interfaces/dfmapplication.h \
    interfaces/private/dfmapplication_p.h \
    controllers/dfmsearchcrumbcontroller.h \
    views/dfmheaderview.h \
    controllers/mountcontroller.h \
    models/mountfileinfo.h \
    views/dfmsplitter.h \
    models/recentfileinfo.h \
    controllers/recentcontroller.h \
    controllers/dfmrecentcrumbcontroller.h \
    views/dfmsidebarrecentitem.h \
    views/dfmsidebarhomeitem.h \
    views/dfmsidebarrootitem.h \
    views/dfmadvancesearchbar.h \
    shutil/dfmregularexpression.h \
    controllers/mergeddesktopcontroller.h \
    models/mergeddesktopfileinfo.h \
    controllers/dfmmdcrumbcontrooler.h \
    interfaces/private/mergeddesktop_common_p.h \
    controllers/masteredmediacontroller.h \
    controllers/dfmmasteredmediacrumbcontroller.h \
    models/masteredmediafileinfo.h \
    views/dfmopticalmediawidget.h \
    dialogs/burnoptdialog.h

SOURCES += \
    controllers/appcontroller.cpp \
    views/dfmsidebaropticaldevitem.cpp \
    views/dtoolbar.cpp \
    views/dfileview.cpp \
    views/ddetailview.cpp \
    views/dicontextbutton.cpp \
    views/dstatebutton.cpp \
    controllers/filecontroller.cpp \
    views/fileitem.cpp \
    models/desktopfileinfo.cpp \
    models/bookmark.cpp \
    models/searchhistory.cpp \
    controllers/bookmarkmanager.cpp \
    dialogs/dialogmanager.cpp \
    controllers/searchhistroymanager.cpp \
    views/windowmanager.cpp \
    shutil/desktopfile.cpp \
    shutil/fileutils.cpp \
    shutil/properties.cpp \
    views/dfilemanagerwindow.cpp \
    views/historystack.cpp\
    dialogs/propertydialog.cpp \
    views/dhoverbutton.cpp \
    views/dfmsidebaritemgroup.cpp \
    controllers/trashmanager.cpp \
    models/trashfileinfo.cpp \
    shutil/mimesappsmanager.cpp \
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
    gvfs/mountaskpassworddialog.cpp \
    gvfs/networkmanager.cpp \
    gvfs/secretmanager.cpp \
    models/networkfileinfo.cpp \
    controllers/networkcontroller.cpp \
    dialogs/trashpropertydialog.cpp \
    controllers/jobcontroller.cpp \
    views/computerview.cpp \
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
    app/define.cpp \
    interfaces/dabstractfilecontroller.cpp \
    interfaces/dabstractfileinfo.cpp \
    interfaces/dfilemenumanager.cpp \
    interfaces/dfileservices.cpp \
    interfaces/dfmevent.cpp \
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
    dialogs/filepreviewdialog.cpp \
    dialogs/usersharepasswordsettingdialog.cpp\
    gvfs/mountsecretdiskaskpassworddialog.cpp \
    views/drenamebar.cpp \
    shutil/filebatchprocess.cpp \
    dialogs/ddesktoprenamedialog.cpp \
    dialogs/dmultifilepropertydialog.cpp \
    dialogs/movetotrashconflictdialog.cpp \
    dialogs/dfmsettingdialog.cpp \
    controllers/operatorrevocation.cpp \
    views/themeconfig.cpp \
    controllers/tagcontroller.cpp \
    models/tagfileinfo.cpp \
    views/dtagactionwidget.cpp \
    views/droundbutton.cpp \
    views/dtagedit.cpp \
    shutil/dsqlitehandle.cpp \
    shutil/danythingmonitorfilter.cpp \
    controllers/tagmanagerdaemoncontroller.cpp \
    controllers/interface/tagmanagerdaemon_interface.cpp \
    interfaces/dfmsettings.cpp \
    interfaces/dfmsidebar.cpp \
    interfaces/dfmsidebaritem.cpp \
    views/dfmsidebaritemseparator.cpp \
    views/dfmsidebardefaultitem.cpp \
    views/dfmsidebarbookmarkitem.cpp \
    views/dfmsidebartrashitem.cpp \
    views/dfmsidebardeviceitem.cpp \
    views/dfmsidebarnetworkitem.cpp \
    views/dfmsidebartagitem.cpp \
    interfaces/dfmcrumbbar.cpp \
    interfaces/dfmcrumbitem.cpp \
    interfaces/dfmcrumbinterface.cpp \
    interfaces/plugins/dfmcrumbplugin.cpp \
    interfaces/plugins/dfmcrumbfactory.cpp \
    interfaces/dfmcrumbmanager.cpp \
    controllers/dfmfilecrumbcontroller.cpp \
    controllers/dfmcomputercrumbcontroller.cpp \
    controllers/dfmbookmarkcrumbcontroller.cpp \
    controllers/dfmtagcrumbcontroller.cpp \
    controllers/dfmnetworkcrumbcontroller.cpp \
    controllers/dfmtrashcrumbcontroller.cpp \
    controllers/dfmusersharecrumbcontroller.cpp \
    controllers/dfmavfscrumbcontroller.cpp \
    controllers/dfmsmbcrumbcontroller.cpp \
    controllers/dfmmtpcrumbcontroller.cpp \
    views/dfmaddressbar.cpp \
    views/dcompleterlistview.cpp \
    interfaces/dfmapplication.cpp \
    controllers/dfmsearchcrumbcontroller.cpp\
    views/dfmheaderview.cpp \
    controllers/mountcontroller.cpp \
    models/mountfileinfo.cpp \
    views/dfmsplitter.cpp \
    models/recentfileinfo.cpp \
    controllers/recentcontroller.cpp \
    controllers/dfmrecentcrumbcontroller.cpp \
    views/dfmsidebarrecentitem.cpp \
    views/dfmsidebarhomeitem.cpp \
    views/dfmsidebarrootitem.cpp \
    views/dfmadvancesearchbar.cpp \
    shutil/dfmregularexpression.cpp \
    models/mergeddesktopfileinfo.cpp \
    controllers/dfmmdcrumbcontrooler.cpp \
    controllers/mergeddesktopcontroller.cpp \
    controllers/masteredmediacontroller.cpp \
    controllers/dfmmasteredmediacrumbcontroller.cpp \
    models/masteredmediafileinfo.cpp \
    views/dfmopticalmediawidget.cpp \
    dialogs/burnoptdialog.cpp

!CONFIG(DISABLE_ANYTHING) {
    HEADERS += shutil/danythingmonitor.h
    SOURCES += shutil/danythingmonitor.cpp
}

APPSHAREDIR = $$PREFIX/share/$$TARGET
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

isEqual(BUILD_MINIMUM, YES){
    templateFiles.files = skin/templates/newTxt.txt
}else{
    templateFiles.files = skin/templates/newDoc.doc \
        skin/templates/newExcel.xls \
        skin/templates/newPowerPoint.ppt \
        skin/templates/newTxt.txt
}

mimetypeFiles.path = $$APPSHAREDIR/mimetypes
mimetypeFiles.files += \
    mimetypes/archive.mimetype \
    mimetypes/text.mimetype \
    mimetypes/video.mimetype \
    mimetypes/audio.mimetype \
    mimetypes/image.mimetype \
    mimetypes/executable.mimetype \
    mimetypes/backup.mimetype

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

icon.path = $$ICONDIR
icon.files = skin/images/$${TARGET}.svg

defaultConfig.path = $$APPSHAREDIR/config
defaultConfig.files = configure/default-view-states.json

INSTALLS += target templateFiles translations mimetypeFiles mimetypeAssociations icon includes includes_private gvfs_includes plugin_includes defaultConfig

DISTFILES += \
    mimetypeassociations/mimetypeassociations.json

include($$PWD/settings_dialog_json.pri)
