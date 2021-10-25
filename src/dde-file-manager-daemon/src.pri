#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T09:14:17
#
#-------------------------------------------------

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/app/filemanagerdaemon.h \
    $$PWD/controllers/appcontroller.h \
    $$PWD/app/global.h \
    $$PWD/controllers/fileoperation.h \
    $$PWD/fileoperationjob/renamejob.h \
    $$PWD/dbusservice/dbusadaptor/fileoperation_adaptor.h \
    $$PWD/dbusservice/dbusadaptor/renamejob_adaptor.h \
    $$PWD/dbusservice/dbusinterface/fileoperation_interface.h \
    $$PWD/dbusservice/dbusinterface/renamejob_interface.h \
    $$PWD/client/filemanagerclient.h \
    $$PWD/dbusservice/dbustype/dbusinforet.h \
    $$PWD/app/policykithelper.h \
    $$PWD/fileoperationjob/basejob.h \
    $$PWD/fileoperationjob/createfolderjob.h \
    $$PWD/fileoperationjob/createtemplatefilejob.h \
    $$PWD/fileoperationjob/movejob.h \
    $$PWD/fileoperationjob/copyjob.h \
    $$PWD/fileoperationjob/deletejob.h \
    $$PWD/dbusservice/dbusadaptor/copyjob_adaptor.h \
    $$PWD/dbusservice/dbusadaptor/createfolderjob_adaptor.h \
    $$PWD/dbusservice/dbusadaptor/createtemplatefilejob_adaptor.h \
    $$PWD/dbusservice/dbusadaptor/deletejob_adaptor.h \
    $$PWD/dbusservice/dbusadaptor/movejob_adaptor.h \
    $$PWD/dbusservice/dbusinterface/copyjob_interface.h \
    $$PWD/dbusservice/dbusinterface/createfolderjob_interface.h \
    $$PWD/dbusservice/dbusinterface/createtemplatefilejob_interface.h \
    $$PWD/dbusservice/dbusinterface/deletejob_interface.h \
    $$PWD/dbusservice/dbusinterface/movejob_interface.h \
    $$PWD/usershare/usersharemanager.h \
    $$PWD/dbusservice/dbusadaptor/usershare_adaptor.h \
    $$PWD/dbusservice/dbusinterface/usershare_interface.h \
    $$PWD/dbusservice/dbusinterface/usbformatter_interface.h \
    $$PWD/tag/tagmanagerdaemon.h \
    $$PWD/dbusservice/dbusadaptor/tagmanagerdaemon_adaptor.h \
    $$PWD/accesscontrol/accesscontrolmanager.h \
    $$PWD/dbusservice/dbusadaptor/accesscontrol_adaptor.h \
    $$PWD/dbusservice/dbusinterface/accesscontrol_interface.h \
    $$PWD/vault/vaultmanager.h \
    $$PWD/dbusservice/dbusadaptor/vault_adaptor.h \
    $$PWD/dbusservice/dbusinterface/vault_interface.h \
    $$PWD/vault/vaultclock.h \
    $$PWD/revocation/revocationmanager.h \
    $$PWD/dbusservice/dbusadaptor/revocationmgr_adaptor.h \
    $$PWD/dbusservice/dbusinterface/revocationmgr_interface.h \
    $$PWD/vault/vaultbruteforceprevention.h \
    $$PWD/dbusservice/dbusadaptor/vaultbruteforceprevention_adaptor.h \
    $$PWD/dbusservice/dbusinterface/vaultbruteforceprevention_interface.h

SOURCES += \
    $$PWD/app/filemanagerdaemon.cpp \
    $$PWD/controllers/appcontroller.cpp \
    $$PWD/controllers/fileoperation.cpp \
    $$PWD/fileoperationjob/renamejob.cpp \
    $$PWD/dbusservice/dbusadaptor/fileoperation_adaptor.cpp \
    $$PWD/dbusservice/dbusadaptor/renamejob_adaptor.cpp \
    $$PWD/dbusservice/dbusinterface/fileoperation_interface.cpp \
    $$PWD/dbusservice/dbusinterface/renamejob_interface.cpp \
    $$PWD/client/filemanagerclient.cpp \
    $$PWD/dbusservice/dbustype/dbusinforet.cpp \
    $$PWD/app/policykithelper.cpp \
    $$PWD/fileoperationjob/basejob.cpp \
    $$PWD/fileoperationjob/createfolderjob.cpp \
    $$PWD/fileoperationjob/createtemplatefilejob.cpp \
    $$PWD/fileoperationjob/movejob.cpp \
    $$PWD/fileoperationjob/copyjob.cpp \
    $$PWD/fileoperationjob/deletejob.cpp \
    $$PWD/dbusservice/dbusadaptor/copyjob_adaptor.cpp \
    $$PWD/dbusservice/dbusadaptor/createfolderjob_adaptor.cpp \
    $$PWD/dbusservice/dbusadaptor/createtemplatefilejob_adaptor.cpp \
    $$PWD/dbusservice/dbusadaptor/deletejob_adaptor.cpp \
    $$PWD/dbusservice/dbusadaptor/movejob_adaptor.cpp \
    $$PWD/dbusservice/dbusinterface/copyjob_interface.cpp \
    $$PWD/dbusservice/dbusinterface/createfolderjob_interface.cpp \
    $$PWD/dbusservice/dbusinterface/createtemplatefilejob_interface.cpp \
    $$PWD/dbusservice/dbusinterface/deletejob_interface.cpp \
    $$PWD/dbusservice/dbusinterface/movejob_interface.cpp \
    $$PWD/usershare/usersharemanager.cpp \
    $$PWD/dbusservice/dbusadaptor/usershare_adaptor.cpp \
    $$PWD/dbusservice/dbusinterface/usershare_interface.cpp \
    $$PWD/dbusservice/dbusinterface/usbformatter_interface.cpp \
    $$PWD/tag/tagmanagerdaemon.cpp \
    $$PWD/dbusservice/dbusadaptor/tagmanagerdaemon_adaptor.cpp \
    $$PWD/accesscontrol/accesscontrolmanager.cpp \
    $$PWD/dbusservice/dbusadaptor/accesscontrol_adaptor.cpp \
    $$PWD/dbusservice/dbusinterface/accesscontrol_interface.cpp \
    $$PWD/vault/vaultmanager.cpp \
    $$PWD/dbusservice/dbusadaptor/vault_adaptor.cpp \
    $$PWD/dbusservice/dbusinterface/vault_interface.cpp \
    $$PWD/vault/vaultclock.cpp \
    $$PWD/revocation/revocationmanager.cpp \
    $$PWD/dbusservice/dbusadaptor/revocationmgr_adaptor.cpp \
    $$PWD/dbusservice/dbusinterface/revocationmgr_interface.cpp \
    $$PWD/vault/vaultbruteforceprevention.cpp \
    $$PWD/dbusservice/dbusadaptor/vaultbruteforceprevention_adaptor.cpp \
    $$PWD/dbusservice/dbusinterface/vaultbruteforceprevention_interface.cpp
