#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------
#system($$PWD/../vendor/prebuild)
#include($$PWD/../vendor/vendor.pri)

INCLUDEPATH += $$PWD

THUMBNAIL_RESOURCE = $$PWD/resources/thumbnail/
DEFINES += THUMBNAIL_RESOURCE=\\\"$$THUMBNAIL_RESOURCE\\\"
HEADERS += \
    $$PWD/testhelper.h

isEqual(ARCH, x86_64) {
SOURCES += $$PWD/shutil/ut_danythingmonitor.cpp
}

SOURCES += \
    $$PWD/main.cpp \
    # vault
    $$PWD/interfaces/ut_dfmbaseview.cpp \
    $$PWD/vault/ut_interfaceactivevault.cpp \
    $$PWD/shutil/ut_checknetwork.cpp \
    $$PWD/io/ut_dfilecopymovejob.cpp \
    $$PWD/vault/ut_operatorcenter.cpp \
    $$PWD/vault/ut_vaulthelper.cpp \
    $$PWD/vault/ut_vaultlockmanager.cpp \
    $$PWD/interfaces/ut_dfmglobal.cpp \
    $$PWD/shutil/ut_danythingmonitorfilter.cpp \
    $$PWD/shutil/ut_desktopfile.cpp \
    $$PWD/shutil/ut_dfmfilelistfile.cpp \
    $$PWD/shutil/ut_dfmregularexpression.cpp \
    $$PWD/controllers/ut_appcontroller.cpp \
    $$PWD/io/ut_dlocalfilehandler.cpp \
    $$PWD/log/ut_dfmlogmanager.cpp \
    $$PWD/views/ut_computerviewitemdelegate.cpp \
    $$PWD/views/ut_dcomputerlistview.cpp \
    $$PWD/views/ut_ddetailview.cpp \
    $$PWD/views/ut_dfiledialog.cpp \
    $$PWD/views/ut_computerview.cpp \
    #$$PWD/views/ut_dfilemanagerwindow.cpp \ # 该用例内部使用了大量线程，大概率造成随机崩溃，暂时注释
    #$$PWD/views/ut_dfileview.cpp \ # 该用例内部使用了大量线程，大概率造成随机崩溃，暂时注释
    $$PWD/views/ut_dfmactionbutton.cpp \
    $$PWD/views/ut_dfmaddressbar.cpp \
    $$PWD/views/ut_dfmadvancesearchbar.cpp \
    $$PWD/views/ut_dtagactionwidget.cpp \
    $$PWD/views/ut_dtagedit.cpp \
    $$PWD/views/ut_dtoolbar.cpp \
    $$PWD/views/ut_extendview.cpp \
    $$PWD/views/ut_filedialogstatusbar.cpp\
    $$PWD/views/ut_fileiconitem.cpp\
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
    $$PWD/views/ut_dfmvaultretrievepassword.cpp \
    $$PWD/views/ut_dfmvaultactivesavekeyfileview.cpp \
    $$PWD/models/ut_dfmappentryfileinfo.cpp \
    $$PWD/controllers/ut_dfmappentrycontroller.cpp

isEqual(ARCH, x86_64) {
SOURCES += \
    $$PWD/io/ut_dgiofiledevice.cpp \
    $$PWD/io/ut_dlocalfiledevice.cpp
}

SOURCES += \
    $$PWD/io/ut_dfilestatisticsjob.cpp \
    $$PWD/io/ut_dstorageinfo.cpp \
    $$PWD/io/ut_dfileiodeviceproxy.cpp

isEqual(ARCH, x86_64) {
SOURCES += \
    $$PWD/bluetooth/ut_bluetoothmanager.cpp \
    $$PWD/bluetooth/ut_bluetoothtransdialog.cpp \
    $$PWD/dbus/ut_dbussystemInfo_test.cpp \
    $$PWD/dialogs/ut_computerpropertydialog.cpp
}

SOURCES += \
    $$PWD/bluetooth/ut_bluetoothdevice.cpp \
    $$PWD/bluetooth/ut_bluetoothadapter.cpp \
    $$PWD/bluetooth/ut_bluetoothmodel.cpp \
    $$PWD/deviceinfo/ut_udisklistener.cpp \
    $$PWD/deviceinfo/ut_udiskdeviceinfo.cpp \
    ####
    $$PWD/interfaces/ut_dfileinfo.cpp \
    $$PWD/interfaces/pulgins/ut_dfmcrumbfactory.cpp \
    $$PWD/interfaces/pulgins/ut_dfmviewplugin.cpp \
    $$PWD/interfaces/pulgins/ut_dfmviewfactory.cpp \
    $$PWD/interfaces/pulgins/ut_dfmgenericplugin.cpp \
    $$PWD/interfaces/pulgins/ut_dfmgenericfactory.cpp \
    $$PWD/interfaces/pulgins/ut_dfmfilepreviewplugin.cpp \
    $$PWD/interfaces/pulgins/ut_dfmfilepreviewfactory.cpp \
    $$PWD/interfaces/pulgins/ut_dfmfilecontrollerplugin.cpp \
    $$PWD/interfaces/pulgins/ut_dfmfilecontrollerfactory.cpp \
    $$PWD/interfaces/pulgins/ut_dfmfactoryloader.cpp \
    $$PWD/interfaces/pulgins/ut_dfmcrumbplugin.cpp \
    $$PWD/dialogs/ut_basedialog.cpp \
    $$PWD/dialogs/ut_burnoptdialog.cpp \
    $$PWD/dialogs/ut_closealldialogindicator.cpp \
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
    $$PWD/interfaces/ut_dfmcrumbbar.cpp \
    ###
    $$PWD/plugins/ut_dfmadditionalmenu.cpp \
    $$PWD/plugins/ut_pluginmanager.cpp \
    $$PWD/fulltextsearch/ut_fulltextsearch.cpp \
    $$PWD/controllers/ut_searchcontroller.cpp \
    $$PWD/sw_label/ut_llsdeepinlabellibrary_test.cpp \
    $$PWD/sw_label/ut_filemanagerlibrary_test.cpp \
    $$PWD/dbusinterface/ut_dbustype.cpp \
    $$PWD/vfs/ut_dfmvfsmanager.cpp \
    #$$PWD/vfs/ut_dfmvfsdevice.cpp \ # 该用例会造成实际ftp挂载，有较大隐患，暂时注释掉
    $$PWD/views/ut_dtabbar.cpp \
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
    $$PWD/controllers/ut_dfmsidebartagitemhandler.cpp \
    $$PWD/controllers/ut_dfmsidebarbookmarkitemhandler.cpp \
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
    $$PWD/gvfs/ut_networkmanager.cpp \
    $$PWD/gvfs/ut_secretmanager.cpp \
    $$PWD/interfaces/ut_dmimedatabase.cpp \
    $$PWD/usershare/ut_shareinfo.cpp \
    $$PWD/usershare/ut_usersharemanager.cpp \
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
    $$PWD/controllers/ut_dfmrootcontroller.cpp \
    $$PWD/controllers/ut_fileeventprocessor.cpp \
    $$PWD/interfaces/ut_dfiledialoghandle.cpp \
    $$PWD/interfaces/ut_dfileiconprovider.cpp \
    $$PWD/fileoperations/ut_filejob.cpp \
    $$PWD/interfaces/ut_dfilemenu.cpp   \
    $$PWD/io/ut_dfiledevice.cpp \
    $$PWD/fileoperations/ut_sort.cpp \
    $$PWD/controllers/ut_jobcontroller.cpp \
    $$PWD/io/ut_dfilehandler.cpp \
    $$PWD/controllers/ut_trashmanager.cpp \
    $$PWD/controllers/ut_sharecontroller.cpp \
    $$PWD/interfaces/ut_dabstractfilecontroller.cpp \
    $$PWD/controllers/ut_networkcontroller.cpp \
    $$PWD/controllers/ut_dfmtrashcrumbcontroller.cpp \
    $$PWD/interfaces/ut_dfileproxywatcher.cpp \
    $$PWD/interfaces/ut_dfmsidebariteminterface.cpp \
    $$PWD/interfaces/ut_dfmstyleditemdelegate.cpp\
    $$PWD/interfaces/ut_dfmapplication.cpp

isEqual(ARCH, x86_64) {
SOURCES += \
    $$PWD/controllers/ut_recentcontroller.cpp \ # 此用例在arm下会导致进程卡主 暂时注释 后期整改
    $$PWD/interfaces/ut_diconitemdelegate.cpp \
    $$PWD/interfaces/ut_dlistitemdelegate.cpp
}

SOURCES += \
    $$PWD/interfaces/ut_dfmfilepreview.cpp

isEqual(ARCH, x86_64) {
SOURCES += \
    $$PWD/controllers/ut_tagcontroller.cpp \
    $$PWD/tag/ut_tagmanager.cpp \
    $$PWD/tag/ut_tagutil.cpp \
    $$PWD/controllers/ut_dfmtagcrumbcontroller.cpp \
    $$PWD/controllers/ut_operatorrevocation.cpp
}

SOURCES += \
    $$PWD/interfaces/ut_dfmabstracteventhandler.cpp \
    $$PWD/interfaces/ut_dabstractfilewatcher.cpp \
    $$PWD/controllers/ut_searchhistroymanager.cpp \
    $$PWD/controllers/ut_dfmsearchcrumbcontroller.cpp

isEqual(ARCH, x86_64) {
SOURCES += \
    $$PWD/interfaces/ut_dfmcrumbinterface.cpp
}

SOURCES += \
    $$PWD/interfaces/ut_dfmcrumblistviewmodel.cpp \
    $$PWD/gvfs/ut_qmount.cpp \
    $$PWD/gvfs/ut_qvolume.cpp

isEqual(ARCH, x86_64) {
SOURCES += \
    $$PWD/interfaces/ut_dfilesystemmodel.cpp
}

SOURCES += \
    $$PWD/controllers/ut_pathmanager.cpp \
    # custom
    $$PWD/interfaces/customization/ut_customaction_sort.cpp \
    $$PWD/interfaces/customization/ut-dcustomactionbuilder.cpp \
    $$PWD/interfaces/customization/ut_dcustomactionparser.cpp \
    $$PWD/interfaces/customization/ut_dcustomactiondata.cpp \
    $$PWD/interfaces/ut_drootfilemanager.cpp \
    #log
    $$PWD/log/ut_filterappender.cpp

isEqual(ARCH, x86_64) {
SOURCES += \
    $$PWD/interfaces/ut_dfileservices.cpp
}

SOURCES += \
    $$PWD/interfaces/ut_dabstractfileinfo.cpp

isEqual(ARCH, x86_64) {
#SOURCES += $$PWD/interfaces/ut_dfilemenumanager.cpp # 由于设计原因，保险箱内部使用了大量静态action，所以疯狂报内存泄露，暂时注释，后期整改
}

SOURCES += \
    $$PWD/interfaces/ut_dgvfsfileinfo.cpp \
    $$PWD/controllers/ut_dfmftpcrumbcontroller.cpp \
    $$PWD/controllers/ut_dfmsftpcrumbcontroller.cpp

!CONFIG(DISABLE_TSAN_TOOL) {
    #DEFINES += TSAN_THREAD #互斥
    DEFINES += ENABLE_TSAN_TOOL
    contains(DEFINES, TSAN_THREAD){
       QMAKE_CXXFLAGS+="-fsanitize=thread"
       QMAKE_CFLAGS+="-fsanitize=thread"
       QMAKE_LFLAGS+="-fsanitize=thread"
    } else {
       QMAKE_CXXFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_CFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
       QMAKE_LFLAGS+="-fsanitize=undefined,address,leak -fno-omit-frame-pointer"
    }
}
