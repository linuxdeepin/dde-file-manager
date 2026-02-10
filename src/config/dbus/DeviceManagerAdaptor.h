// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEMANAGERADAPTOR_H
#define DEVICEMANAGERADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.deepin.Filemanager.Daemon.DeviceManager
 */
class DeviceManagerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.DeviceManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.Filemanager.Daemon.DeviceManager\">\n"
"    <signal name=\"SizeUsedChanged\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"x\" name=\"total\"/>\n"
"      <arg direction=\"out\" type=\"x\" name=\"free\"/>\n"
"    </signal>\n"
"    <signal name=\"NotifyDeviceBusy\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"action\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDriveAdded\"/>\n"
"    <signal name=\"BlockDriveRemoved\"/>\n"
"    <signal name=\"BlockDeviceAdded\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDeviceRemoved\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"oldMpt\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDeviceFilesystemAdded\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDeviceFilesystemRemoved\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDeviceMounted\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"mountPoint\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDeviceUnmounted\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"oldMpt\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDevicePropertyChanged\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"property\"/>\n"
"      <arg direction=\"out\" type=\"v\" name=\"value\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDeviceUnlocked\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"clearDeviceId\"/>\n"
"    </signal>\n"
"    <signal name=\"BlockDeviceLocked\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"    </signal>\n"
"    <signal name=\"ProtocolDeviceAdded\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"    </signal>\n"
"    <signal name=\"ProtocolDeviceRemoved\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"oldMpt\"/>\n"
"    </signal>\n"
"    <signal name=\"ProtocolDeviceMounted\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"mountPoint\"/>\n"
"    </signal>\n"
"    <signal name=\"ProtocolDeviceUnmounted\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"oldMpt\"/>\n"
"    </signal>\n"
"    <method name=\"IsMonotorWorking\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"DetachBlockDevice\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"id\"/>\n"
"    </method>\n"
"    <method name=\"DetachProtocolDevice\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"id\"/>\n"
"    </method>\n"
"    <method name=\"DetachAllMountedDevices\"/>\n"
"    <method name=\"GetBlockDevicesIdList\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"opts\"/>\n"
"    </method>\n"
"    <method name=\"QueryBlockDeviceInfo\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"reload\"/>\n"
"    </method>\n"
"    <method name=\"GetProtocolDevicesIdList\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"QueryProtocolDeviceInfo\">\n"
"      <arg direction=\"out\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"id\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"reload\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    DeviceManagerAdaptor(QObject *parent);
    virtual ~DeviceManagerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void DetachAllMountedDevices();
    void DetachBlockDevice(const QString &id);
    void DetachProtocolDevice(const QString &id);
    QStringList GetBlockDevicesIdList(int opts);
    QStringList GetProtocolDevicesIdList();
    bool IsMonotorWorking();
    QVariantMap QueryBlockDeviceInfo(const QString &id, bool reload);
    QVariantMap QueryProtocolDeviceInfo(const QString &id, bool reload);
Q_SIGNALS: // SIGNALS
    void BlockDeviceAdded(const QString &id);
    void BlockDeviceFilesystemAdded(const QString &id);
    void BlockDeviceFilesystemRemoved(const QString &id);
    void BlockDeviceLocked(const QString &id);
    void BlockDeviceMounted(const QString &id, const QString &mountPoint);
    void BlockDevicePropertyChanged(const QString &id, const QString &property, const QDBusVariant &value);
    void BlockDeviceRemoved(const QString &id, const QString &oldMpt);
    void BlockDeviceUnlocked(const QString &id, const QString &clearDeviceId);
    void BlockDeviceUnmounted(const QString &id, const QString &oldMpt);
    void BlockDriveAdded();
    void BlockDriveRemoved();
    void NotifyDeviceBusy(const QString &id, const QString &action);
    void ProtocolDeviceAdded(const QString &id);
    void ProtocolDeviceMounted(const QString &id, const QString &mountPoint);
    void ProtocolDeviceRemoved(const QString &id, const QString &oldMpt);
    void ProtocolDeviceUnmounted(const QString &id, const QString &oldMpt);
    void SizeUsedChanged(const QString &id, qlonglong total, qlonglong free);
};

#endif
