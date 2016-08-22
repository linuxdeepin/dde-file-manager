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

PKGCONFIG += x11 polkit-agent-1 polkit-qt5-1
CONFIG += c++11 link_pkgconfig

TEMPLATE = app

FileOperation.files = \
    dbusservice/fileoperation.xml

FileOperation.header_flags += -i fileoperation.h -c FileOperationAdaptor -l FileOperation
FileOperation.source_flags += -i fileoperation.h -c FileOperationAdaptor -l FileOperation


RenameJob.files = \
    dbusservice/renamejob.xml

RenameJob.header_flags += -i renamejob.h -c RenameJobAdaptor -l RenameJob
RenameJob.source_flags += -i renamejob.h -c RenameJobAdaptor -l RenameJob


DBUS_ADAPTORS += FileOperation RenameJob


SOURCES += main.cpp \
    fileoperation.cpp \
    renamejob.cpp

HEADERS += \
    fileoperation.h \
    renamejob.h


target.path = /usr/bin

poliktpolicy.path = /usr/share/polkit-1/actions
poliktpolicy.files = dbusservice/com.deepin.pkexec.filemanager.daemon.policy

systembusconf.path = /etc/dbus-1/system.d
systembusconf.files = dbusservice/com.deepin.filemanager.daemon.conf

services.path = /usr/share/dbus-1/system-services
services.files = dbusservice/com.deepin.filemanager.daemon.service


INSTALLS += target poliktpolicy systembusconf services
