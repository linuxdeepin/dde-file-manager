// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/DeviceManagerAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class DeviceManagerAdaptor
 */

DeviceManagerAdaptor::DeviceManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DeviceManagerAdaptor::~DeviceManagerAdaptor()
{
    // destructor
}

void DeviceManagerAdaptor::DetachAllMountedDevices()
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.DetachAllMountedDevices
    QMetaObject::invokeMethod(parent(), "DetachAllMountedDevices");
}

void DeviceManagerAdaptor::DetachBlockDevice(const QString &id)
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.DetachBlockDevice
    QMetaObject::invokeMethod(parent(), "DetachBlockDevice", Q_ARG(QString, id));
}

void DeviceManagerAdaptor::DetachProtocolDevice(const QString &id)
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.DetachProtocolDevice
    QMetaObject::invokeMethod(parent(), "DetachProtocolDevice", Q_ARG(QString, id));
}

QStringList DeviceManagerAdaptor::GetBlockDevicesIdList(int opts)
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.GetBlockDevicesIdList
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "GetBlockDevicesIdList", Q_RETURN_ARG(QStringList, out0), Q_ARG(int, opts));
    return out0;
}

QStringList DeviceManagerAdaptor::GetProtocolDevicesIdList()
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.GetProtocolDevicesIdList
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "GetProtocolDevicesIdList", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

bool DeviceManagerAdaptor::IsMonotorWorking()
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.IsMonotorWorking
    bool out0;
    QMetaObject::invokeMethod(parent(), "IsMonotorWorking", Q_RETURN_ARG(bool, out0));
    return out0;
}

QVariantMap DeviceManagerAdaptor::QueryBlockDeviceInfo(const QString &id, bool reload)
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.QueryBlockDeviceInfo
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "QueryBlockDeviceInfo", Q_RETURN_ARG(QVariantMap, out0), Q_ARG(QString, id), Q_ARG(bool, reload));
    return out0;
}

QVariantMap DeviceManagerAdaptor::QueryProtocolDeviceInfo(const QString &id, bool reload)
{
    // handle method call org.deepin.Filemanager.Daemon.DeviceManager.QueryProtocolDeviceInfo
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "QueryProtocolDeviceInfo", Q_RETURN_ARG(QVariantMap, out0), Q_ARG(QString, id), Q_ARG(bool, reload));
    return out0;
}

