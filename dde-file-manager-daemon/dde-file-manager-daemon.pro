#-------------------------------------------------
#
# Project created by QtCreator 2016-08-02T13:45:46
#
#-------------------------------------------------

QT       += core dbus

QT       -= gui

TARGET = dde-file-manager-daemon
CONFIG   += console
CONFIG   -= app_bundle

PKGCONFIG += x11 polkit-agent-1 polkit-qt5-1 dtkbase dtkutil
CONFIG += c++11 link_pkgconfig

TEMPLATE = app

#include(../utils/utils.pri)
#include(../fileoperations/fileoperations.pri)

INCLUDEPATH += dbusservice

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
    dbusservice/dbusinterface/movejob_interface.cpp

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
    dbusservice/dbusinterface/movejob_interface.h


target.path = /usr/bin

poliktpolicy.path = /usr/share/polkit-1/actions
poliktpolicy.files = dbusservice/com.deepin.filemanager.daemon.policy

systembusconf.path = /etc/dbus-1/system.d
systembusconf.files = dbusservice/com.deepin.filemanager.daemon.conf

services.path = /usr/share/dbus-1/system-services
services.files = dbusservice/com.deepin.filemanager.daemon.service


INSTALLS += target poliktpolicy systembusconf services

DISTFILES += \
    dbusservice/fileoperation.xml \
    dbusservice/renamejob.xml
