/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "mountutils.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <ddiskdevice.h>
#include <dblockdevice.h>
#include <ddiskmanager.h>

#define SE_BusName "com.deepin.daemon.SecurityEnhance"
#define SE_ObjectPath "/com/deepin/daemon/SecurityEnhance"
#define SE_Interface SE_BusName

#define SE_Removable 1
#define SE_Native 2

QString MountUtils::getSeLinuxMountParam(bool removable)
{
    if (!isSecurityEnhanceOpen()) {
        qInfo() << "SE status is not open";
        return "";
    }

    QDBusInterface iface(SE_BusName, SE_ObjectPath, SE_Interface, QDBusConnection::systemBus());
    if (!iface.isValid()) {
        qWarning() << "SE interface is not valid";
        return "";
    }

    qDebug() << "start get se mount param;";
    int deviceType = removable ? SE_Removable : SE_Native;
    QDBusReply<QString> reply = iface.call("GetLabel", deviceType);
    if (!reply.isValid()) {
        qWarning() << "invoke GetLabel failed: " << reply.error();
        return "";
    }
    qDebug() << "end get se mount param;";

    return reply.value();
}

bool MountUtils::isSecurityEnhanceOpen()
{
    QDBusInterface iface(SE_BusName, SE_ObjectPath, SE_Interface, QDBusConnection::systemBus());
    if (!iface.isValid()) {
        qWarning() << "SE interface is not valid";
        return false;
    }

    qDebug() << "start get se status;";
    QDBusReply<QString> reply = iface.call("Status");
    if (!reply.isValid()) {
        qWarning() << "invoke Status failed: " << reply.error();
        return false;
    }
    qDebug() << "end get se status;";

    return reply.value() == "open";
}

QVariantMap MountUtils::getSeLinuxMountParams(bool removable)
{
    QString seLinuxTag = getSeLinuxMountParam(removable);
    if (seLinuxTag.isEmpty())
        return {};

    QVariantMap params;
    params.insert("options", seLinuxTag);
    return params;
}

QString MountUtils::mountBlkWithParams(DBlockDevice *dev)
{
    if (!dev) {
        qWarning() << "dev is not valid!";
        return "";
    }

    QVariantMap params;
    if (dev->idType().startsWith("ext")) {
        bool removable = true;
        QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(dev->drive()));
        if (!diskDev)
            qWarning() << "cannot create disk device, treat like removable device";
        else
            removable = diskDev->removable();

        params = getSeLinuxMountParams(removable);
        qInfo() << "mount device: " << dev->device() << "with params: " << params;
    }
    return dev->mount(params);
}
