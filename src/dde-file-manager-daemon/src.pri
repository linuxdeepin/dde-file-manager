#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

3RD_DBUSSERVICE = $$PWD/../../3rdparty/dbusservice
INCLUDEPATH += $$PWD \
               $$3RD_DBUSSERVICE

HEADERS += \
    $$PWD/app/filemanagerdaemon.h \
    $$PWD/controllers/appcontroller.h \
    $$PWD/app/global.h \
    $$PWD/controllers/fileoperation.h \
    $$PWD/fileoperationjob/renamejob.h \
    $$PWD/client/filemanagerclient.h \
    $$PWD/dbusservice/dbustype/dbusinforet.h \
    $$PWD/app/policykithelper.h \
    $$PWD/fileoperationjob/basejob.h \
    $$PWD/fileoperationjob/createfolderjob.h \
    $$PWD/fileoperationjob/createtemplatefilejob.h \
    $$PWD/fileoperationjob/movejob.h \
    $$PWD/fileoperationjob/copyjob.h \
    $$PWD/fileoperationjob/deletejob.h \
    $$PWD/usershare/usersharemanager.h \
    $$PWD/tag/tagmanagerdaemon.h \
    $$PWD/accesscontrol/accesscontrolmanager.h \
    $$PWD/vault/vaultmanager.h \
    $$PWD/vault/vaultclock.h \
    $$PWD/revocation/revocationmanager.h \
    $$PWD/vault/vaultbruteforceprevention.h \
    $$PWD/anything/anything.h \
    $$PWD/disk/diskmanager.h \
    $$3RD_DBUSSERVICE/dbusadaptor/accesscontrol_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/copyjob_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/createfolderjob_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/createtemplatefilejob_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/deletejob_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/disk_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/fileoperation_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/movejob_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/renamejob_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/revocationmgr_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/tagmanagerdaemon_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/usershare_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/vault_adaptor.h \
    $$3RD_DBUSSERVICE/dbusadaptor/vaultbruteforceprevention_adaptor.h \
    $$3RD_DBUSSERVICE/dbusinterface/accesscontrol_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/copyjob_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/createfolderjob_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/createtemplatefilejob_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/deletejob_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/disk_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/fileoperation_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/movejob_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/renamejob_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/revocationmgr_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/usbformatter_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/usershare_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/vault_interface.h \
    $$3RD_DBUSSERVICE/dbusinterface/vaultbruteforceprevention_interface.h

SOURCES += \
    $$PWD/app/filemanagerdaemon.cpp \
    $$PWD/controllers/appcontroller.cpp \
    $$PWD/controllers/fileoperation.cpp \
    $$PWD/fileoperationjob/renamejob.cpp \
    $$PWD/client/filemanagerclient.cpp \
    $$PWD/dbusservice/dbustype/dbusinforet.cpp \
    $$PWD/app/policykithelper.cpp \
    $$PWD/fileoperationjob/basejob.cpp \
    $$PWD/fileoperationjob/createfolderjob.cpp \
    $$PWD/fileoperationjob/createtemplatefilejob.cpp \
    $$PWD/fileoperationjob/movejob.cpp \
    $$PWD/fileoperationjob/copyjob.cpp \
    $$PWD/fileoperationjob/deletejob.cpp \
    $$PWD/usershare/usersharemanager.cpp \
    $$PWD/tag/tagmanagerdaemon.cpp \
    $$PWD/accesscontrol/accesscontrolmanager.cpp \
    $$PWD/vault/vaultmanager.cpp \
    $$PWD/vault/vaultclock.cpp \
    $$PWD/revocation/revocationmanager.cpp \
    $$PWD/vault/vaultbruteforceprevention.cpp \
    $$PWD/anything/anything.cpp \
    $$PWD/disk/diskmanager.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/accesscontrol_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/copyjob_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/createfolderjob_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/createtemplatefilejob_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/deletejob_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/disk_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/fileoperation_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/movejob_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/renamejob_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/revocationmgr_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/tagmanagerdaemon_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/usershare_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/vault_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusadaptor/vaultbruteforceprevention_adaptor.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/accesscontrol_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/copyjob_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/createfolderjob_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/createtemplatefilejob_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/deletejob_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/disk_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/fileoperation_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/movejob_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/renamejob_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/revocationmgr_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/usbformatter_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/usershare_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/vault_interface.cpp \
    $$3RD_DBUSSERVICE/dbusinterface/vaultbruteforceprevention_interface.cpp
