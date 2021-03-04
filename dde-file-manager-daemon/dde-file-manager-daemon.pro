#-------------------------------------------------
#
# Project created by QtCreator 2016-08-02T13:45:46
#
#-------------------------------------------------

QT       += core dbus concurrent

QT       -= gui

TARGET = dde-file-manager-daemon
CONFIG   += console
CONFIG   -= app_bundle

PKGCONFIG += x11 polkit-agent-1 polkit-qt5-1 udisks2-qt5 libcryptsetup
CONFIG(release, release|debug) {
    PKGCONFIG += dtkwidget
} else {
    PKGCONFIG += dtkwidget
}
CONFIG += c++11 link_pkgconfig

LIBS += -L$$OUT_PWD/../dde-file-manager-lib -ldde-file-manager -lKF5Codecs

CONFIG(debug, debug|release) {
    DEPENDPATH += $$PWD/../dde-file-manager-lib
    unix:QMAKE_RPATHDIR += $$OUT_PWD/../dde-file-manager-lib
}

TEMPLATE = app

#include(../utils/utils.pri)
#include(../fileoperations/fileoperations.pri)

INCLUDEPATH += dbusservice
INCLUDEPATH += $$PWD/../dde-file-manager-lib $$PWD/.. \
               $$PWD/../utils \
               $$PWD/../dde-file-manager-lib/interfaces \
               $$PWD/../dde-file-manager-lib/shutil

DEFINES += QT_MESSAGELOGCONTEXT


SOURCES += main.cpp \
    app/filemanagerdaemon.cpp \
    controllers/appcontroller.cpp \
    controllers/fileoperation.cpp \
    fileoperationjob/renamejob.cpp \
    dbusservice/dbusadaptor/fileoperation_adaptor.cpp \
    dbusservice/dbusadaptor/renamejob_adaptor.cpp \
    dbusservice/dbusinterface/fileoperation_interface.cpp \
    dbusservice/dbusinterface/renamejob_interface.cpp \
    client/filemanagerclient.cpp \
    dbusservice/dbustype/dbusinforet.cpp \
    app/policykithelper.cpp \
    fileoperationjob/basejob.cpp \
    fileoperationjob/createfolderjob.cpp \
    fileoperationjob/createtemplatefilejob.cpp \
    fileoperationjob/movejob.cpp \
    fileoperationjob/copyjob.cpp \
    fileoperationjob/deletejob.cpp \
    dbusservice/dbusadaptor/copyjob_adaptor.cpp \
    dbusservice/dbusadaptor/createfolderjob_adaptor.cpp \
    dbusservice/dbusadaptor/createtemplatefilejob_adaptor.cpp \
    dbusservice/dbusadaptor/deletejob_adaptor.cpp \
    dbusservice/dbusadaptor/movejob_adaptor.cpp \
    dbusservice/dbusinterface/copyjob_interface.cpp \
    dbusservice/dbusinterface/createfolderjob_interface.cpp \
    dbusservice/dbusinterface/createtemplatefilejob_interface.cpp \
    dbusservice/dbusinterface/deletejob_interface.cpp \
    dbusservice/dbusinterface/movejob_interface.cpp \
    usershare/usersharemanager.cpp \
    dbusservice/dbusadaptor/usershare_adaptor.cpp \
    dbusservice/dbusinterface/usershare_interface.cpp \
    dbusservice/dbusinterface/usbformatter_interface.cpp \
    tag/tagmanagerdaemon.cpp \
    dbusservice/dbusadaptor/tagmanagerdaemon_adaptor.cpp \
    acesscontrol/acesscontrolmanager.cpp \
    dbusservice/dbusadaptor/acesscontrol_adaptor.cpp \
    dbusservice/dbusinterface/acesscontrol_interface.cpp \
    vault/vaultmanager.cpp \
    dbusservice/dbusadaptor/vault_adaptor.cpp \
    dbusservice/dbusinterface/vault_interface.cpp \
    vault/vaultclock.cpp \
    disk/diskmanager.cpp \
    dbusservice/dbusadaptor/disk_adaptor.cpp \
    dbusservice/dbusinterface/disk_interface.cpp

HEADERS += \
    app/filemanagerdaemon.h \
    controllers/appcontroller.h \
    app/global.h \
    controllers/fileoperation.h \
    fileoperationjob/renamejob.h \
    dbusservice/dbusadaptor/fileoperation_adaptor.h \
    dbusservice/dbusadaptor/renamejob_adaptor.h \
    dbusservice/dbusinterface/fileoperation_interface.h \
    dbusservice/dbusinterface/renamejob_interface.h \
    client/filemanagerclient.h \
    dbusservice/dbustype/dbusinforet.h \
    app/policykithelper.h \
    fileoperationjob/basejob.h \
    fileoperationjob/createfolderjob.h \
    fileoperationjob/createtemplatefilejob.h \
    fileoperationjob/movejob.h \
    fileoperationjob/copyjob.h \
    fileoperationjob/deletejob.h \
    dbusservice/dbusadaptor/copyjob_adaptor.h \
    dbusservice/dbusadaptor/createfolderjob_adaptor.h \
    dbusservice/dbusadaptor/createtemplatefilejob_adaptor.h \
    dbusservice/dbusadaptor/deletejob_adaptor.h \
    dbusservice/dbusadaptor/movejob_adaptor.h \
    dbusservice/dbusinterface/copyjob_interface.h \
    dbusservice/dbusinterface/createfolderjob_interface.h \
    dbusservice/dbusinterface/createtemplatefilejob_interface.h \
    dbusservice/dbusinterface/deletejob_interface.h \
    dbusservice/dbusinterface/movejob_interface.h \
    usershare/usersharemanager.h \
    dbusservice/dbusadaptor/usershare_adaptor.h \
    dbusservice/dbusinterface/usershare_interface.h \
    dbusservice/dbusinterface/usbformatter_interface.h \
    tag/tagmanagerdaemon.h \
    dbusservice/dbusadaptor/tagmanagerdaemon_adaptor.h \
    acesscontrol/acesscontrolmanager.h \
    dbusservice/dbusadaptor/acesscontrol_adaptor.h \
    dbusservice/dbusinterface/acesscontrol_interface.h \
    vault/vaultmanager.h \
    dbusservice/dbusadaptor/vault_adaptor.h \
    dbusservice/dbusinterface/vault_interface.h \
    vault/vaultclock.h \
    disk/diskmanager.h \
    dbusservice/dbusadaptor/disk_adaptor.h \
    dbusservice/dbusinterface/disk_interface.h

target.path = /usr/bin

poliktpolicy.path = /usr/share/polkit-1/actions
poliktpolicy.files = dbusservice/com.deepin.filemanager.daemon.policy

systembusconf.path = /etc/dbus-1/system.d
systembusconf.files = dbusservice/com.deepin.filemanager.daemon.conf

services.path = /usr/share/dbus-1/system-services
services.files = dbusservice/com.deepin.filemanager.daemon.service

systemd_service.files = dbusservice/dde-filemanager-daemon.service
systemd_service.path = /lib/systemd/system

INSTALLS += target poliktpolicy systembusconf services systemd_service

DISTFILES += \
    dbusservice/fileoperation.xml \
    dbusservice/renamejob.xml
