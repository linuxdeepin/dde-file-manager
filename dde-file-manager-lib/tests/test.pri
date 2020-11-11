#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

INCLUDEPATH += $$PWD

HEADERS += \

SOURCES += \
    $$PWD/main.cpp \
    # vault
    $$PWD/vault/ut_interfaceactivevault.cpp \
    $$PWD/vault/ut_operatorcenter.cpp \
    $$PWD/vault/ut_vaulthelper.cpp \
    $$PWD/vault/ut_vaultlockmanager.cpp \
    $$PWD/interfaces/ut_dfmglobal.cpp \
    $$PWD/shutil/ut_danythingmonitor.cpp \
    $$PWD/shutil/ut_danythingmonitorfilter.cpp \
    $$PWD/shutil/ut_desktopfile.cpp \
    $$PWD/shutil/ut_dfmfilelistfile.cpp \
    $$PWD/shutil/ut_dfmregularexpression.cpp \
    $$PWD/controllers/ut_appcontroller.cpp \
    $$PWD/io/ut_dlocalfilehandler.cpp \
    $$PWD/views/ut_computerviewitemdelegate.cpp \
    $$PWD/views/ut_dcomputerlistview.cpp \
    $$PWD/views/ut_ddetailview.cpp \
    $$PWD/views/ut_dfiledialog.cpp \
    $$PWD/views/ut_computerview.cpp \
    $$PWD/views/ut_dfilemanagerwindow.cpp \
    $$PWD/views/ut_dfileview.cpp \
    $$PWD/views/ut_dfmactionbutton.cpp \
    $$PWD/views/ut_dfmaddressbar.cpp \
    $$PWD/views/ut_dfmadvancesearchbar.cpp \
    $$PWD/views/ut_dtagactionwidget.cpp \
    $$PWD/views/ut_dtagedit.cpp \
    $$PWD/views/ut_dtoolbar.cpp \
    $$PWD/views/ut_extendview.cpp \
    $$PWD/views/ut_filedialogstatusbar.cpp\
    $$PWD/views/ut_fileitem.cpp\
    $$PWD/views/ut_fileviewhelper.cpp\
    $$PWD/views/ut_historystack.cpp\
    $$PWD/views/ut_windowmanager.cpp\
    $$PWD/controllers/ut_masteredmediacontroller.cpp \
    $$PWD/controllers/ut_mergeddesktopcontroller.cpp \
    $$PWD/controllers/ut_mountcontroller.cpp \
    $$PWD/models/ut_avfsfileinfo.cpp \
    $$PWD/models/ut_bookmark.cpp \
    $$PWD/models/ut_computermodel.cpp \
    $$PWD/models/ut_desktopfileinfo.cpp \
    $$PWD/models/ut_deviceinfoparser.cpp \
    $$PWD/models/ut_dfileselectionmodel.cpp \
    $$PWD/models/ut_dfmrootfileinfo.cpp \
    $$PWD/models/ut_dfmsidebarmodel.cpp \
    $$PWD/models/ut_masteredmediafileinfo.cpp \
    $$PWD/models/ut_mergeddesktopfileinfo.cpp \
    $$PWD/models/ut_mountfileinfo.cpp \
    $$PWD/models/ut_networkfileinfo.cpp \
    $$PWD/models/ut_recentfileinfo.cpp \
    $$PWD/models/ut_searchfileinfo.cpp \
    $$PWD/models/ut_searchhistory.cpp \
    $$PWD/models/ut_sharefileinfo.cpp \
    $$PWD/models/ut_tagfileinfo.cpp \
    $$PWD/models/ut_trashfileinfo.cpp \
    $$PWD/mediainfo/ut_dfmmediainfo.cpp \
    $$PWD/io/ut_dfilecopyqueue.cpp \
    $$PWD/io/ut_dgiofiledevice.cpp \
    $$PWD/io/ut_dlocalfiledevice.cpp \
    $$PWD/io/ut_dfilestatisticsjob.cpp \
    $$PWD/io/ut_dstorageinfo.cpp \
    $$PWD/io/ut_dfilecopymovejob.cpp \
    $$PWD/io/ut_dfileiodeviceproxy.cpp \
    $$PWD/bluetooth/ut_bluetoothdevice.cpp \
    $$PWD/bluetooth/ut_bluetoothadapter.cpp \
    $$PWD/bluetooth/ut_bluetoothmanager.cpp \
    $$PWD/bluetooth/ut_bluetoothmodel.cpp \
    $$PWD/deviceinfo/ut_udisklistener.cpp \
    $$PWD/deviceinfo/ut_udiskdeviceinfo.cpp \
    $$PWD/bluetooth/ut_bluetoothtransdialog.cpp \
    $$PWD/dialogs/ut_basedialog.cpp \
    $$PWD/dialogs/ut_burnoptdialog.cpp \
    $$PWD/dialogs/ut_closealldialogindicator.cpp \
    $$PWD/dialogs/ut_computerpropertydialog.cpp \
    $$PWD/dialogs/ut_connecttoserverdialog.cpp \
    $$PWD/dialogs/ut_ddeskprenamedialog.cpp \
    $$PWD/dialogs/ut_dfmsettingdialog.cpp \
    $$PWD/dialogs/ut_dialogmanager.cpp \
    $$PWD/dialogs/ut_dmultifilepropertydialog.cpp \
    $$PWD/dialogs/ut_filepreviewdialog.cpp \
    $$PWD/dialogs/ut_movetotrashconflictdialog.cpp \
    $$PWD/dialogs/ut_openwithdialog.cpp \
    $$PWD/dialogs/ut_propertydialog.cpp \
    $$PWD/dialogs/ut_shareinfoframe.cpp \
    $$PWD/dialogs/ut_trashpropertydialog.cpp \
    $$PWD/dialogs/ut_usersharepasswordsettingdialog.cpp \
    $$PWD/plugins/ut_dfmadditionalmenu.cpp \
    $$PWD/plugins/ut_pluginmanager.cpp \
    $$PWD/fulltextsearch/ut_fulltextsearch.cpp \
    $$PWD/controllers/ut_searchcontroller.cpp \
    $$PWD/dbus/ut_dbussystemInfo_test.cpp \
    $$PWD/sw_label/ut_llsdeepinlabellibrary_test.cpp \
    $$PWD/sw_label/ut_filemanagerlibrary_test.cpp \
    $$PWD/dbusinterface/ut_dbustype.cpp \
    $$PWD/vfs/ut_dfmvfsmanager.cpp \
    $$PWD/vfs/ut_dfmvfsdevice.cpp \
    $$PWD/views/ut_dstatusbar.cpp \
    $$PWD/views/ut_droundbutton.cpp \
    $$PWD/views/ut_drenamebar.cpp \
    $$PWD/views/ut_dfmrightdetailview.cpp \
    $$PWD/views/ut_dfmopticalmediawidget.cpp \
    $$PWD/views/ut_dfmheaderview.cpp \
    $$PWD/views/ut_dfmfilebasicinfowidget.cpp \
    $$PWD/views/ut_dfmtagwidget.cpp \
    $$PWD/views/ut_dfmsplitter.cpp \
    $$PWD/views/ut_dfmsidebaritemdelegate.cpp \
    $$PWD/views/ut_dfmsidebarview.cpp \
    $$PWD/views/ut_dfmvaultactivefinishedview.cpp \
    $$PWD/views/ut_dfmvaultactivesavekeyview.cpp \
    $$PWD/views/ut_dfmvaultactivesetunlockmethodview.cpp \
    $$PWD/views/ut_dfmvaultactivestartview.cpp \
    $$PWD/views/ut_dfmvaultactiveview.cpp \
    $$PWD/views/ut_dfmvaultfileview.cpp \
    $$PWD/views/ut_dfmvaultpagebase.cpp \
    $$PWD/views/ut_dfmvaultrecoverykeypages.cpp \
    $$PWD/views/ut_dfmvaultremovebypasswordview.cpp \
    $$PWD/views/ut_dfmvaultremovebyrecoverykeyview.cpp \
    $$PWD/views/ut_dfmvaultremovepages.cpp \
    $$PWD/views/ut_dfmvaultremoveprogressview.cpp \
    $$PWD/views/ut_dfmvaultunlockpages.cpp \
    $$PWD/controllers/ut_vaulthandle.cpp \
    $$PWD/controllers/ut_vaultcontroller.cpp \
    $$PWD/controllers/ut_dfmsidebarvaultitemhandler.cpp \
    $$PWD/controllers/ut_dfmvaultcrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmsmbcrumbcontroller.cpp \
    $$PWD/chinese2pinyin/ut-chinese2pinyin-test.cpp \
    $$PWD/controllers/ut_dfmsidebardefaultitemhandler_test.cpp \
    $$PWD/controllers/ut_dfmsidebardeviceitemhandler_test.cpp \
    $$PWD/controllers/ut_tagmanagerdaemon_interface.cpp \
    $$PWD/controllers/ut_dfmavfscrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmbookmarkcrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmcomputercrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmfilecrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmmasteredmediacrumbcontroller.cpp \
    $$PWD/shutil/ut_filebatchprocess.cpp \
    $$PWD/shutil/ut_fileutils.cpp \
    $$PWD/shutil/ut_dsqlitehandle.cpp \
    $$PWD/shutil/ut_mimesappsmanager.cpp \
    $$PWD/shutil/ut_mimetypedisplaymanager.cpp \
    $$PWD/shutil/ut_properties.cpp \
    $$PWD/shutil/ut_shortcut.cpp \
    $$PWD/models/ut_vaultfileinfo.cpp \
    $$PWD/interfaces/ut_dfmsettings.cpp \
    $$PWD/interfaces/ut_dfmviewmanager.cpp \
    $$PWD/controllers/ut_bookmarkmanager.cpp \
    $$PWD/gvfs/ut_mountaskpassworddialog.cpp \
    $$PWD/gvfs/ut_mountsecretdiskaskpassworddialog.cpp \
    $$PWD/gvfs/ut_qdrive.cpp \
    $$PWD/gvfs/ut_gvfsmountmanager.cpp \
    $$PWD/gvfs/ut_secretmanager.cpp \
    $$PWD/interfaces/ut_dmimedatabase.cpp \
    $$PWD/usershare/ut_shareinfo.cpp \
    $$PWD/usershare/ut_usersharemanager.cpp \
    $$PWD/controllers/ut_networkcontroller.cpp \
    $$PWD/controllers/ut_dfmusersharecrumbcontroller.cpp \
    $$PWD/interfaces/ut_dfmsidebarmanager.cpp \
    $$PWD/interfaces/ut_dthumbnailprovider.cpp \
    $$PWD/interfaces/ut_dfmcrumbmanager.cpp \
    $$PWD/interfaces/ut_dfmevent.cpp \
    $$PWD/interfaces/ut_dfmstandardpaths.cpp \
    $$PWD/app/ut_FileSignalManager.cpp  \
    $$PWD/controllers/ut_dfmmdcrumbcontrooler.cpp \
    $$PWD/controllers/ut_dfmmtpcrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmnetworkcrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmrecentcrumbcontroller.cpp \
    $$PWD/controllers/ut_filecontroller.cpp \
    $$PWD/interfaces/ut_dfiledialoghandle.cpp \
    $$PWD/interfaces/ut_dfileiconprovider.cpp

RESOURCES += \
    $$PWD/resources/test.qrc




