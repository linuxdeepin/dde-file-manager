/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "deviceinfomanager.h"
#include "dbusadaptor/deviceinfomanager_adaptor.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QtConcurrent>
#include <QThreadPool>
#include <QFuture>
#include <QDebug>

QString DeviceInfoManager::ObjectPath = "/com/deepin/filemanager/daemon/DeviceInfoManager";
DeviceInfoManager::DeviceInfoManager(QObject *parent) : QObject(parent)
{
    PartMan::Partition::registerMetaType();
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_deviceInfoManagerAdaptor = new DeviceInfoManagerAdaptor(this);
    m_readUsageManager = new PartMan::ReadUsageManager(this);
}

PartMan::Partition DeviceInfoManager::getPartitionByDevicePath(const QString &devicePath)
{
    PartMan::Partition p = PartMan::Partition::getPartitionByDevicePath(devicePath);
    qDebug() << p;
    return p;
}

bool DeviceInfoManager::readUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
//    typedef bool (PartMan::ReadUsageManager::*readUsageFun) (const QString&, qlonglong& , qlonglong&);
//    readUsageFun f = &PartMan::ReadUsageManager::readUsage;
//    QFuture<bool> future = QtConcurrent::run(m_readUsageManager,
//                                             f,
//                                             path, freespace, total);
//    future.waitForFinished();
//    bool ret = future.result();
    bool ret = m_readUsageManager->readUsage(path , freespace, total);
    return ret;
}
