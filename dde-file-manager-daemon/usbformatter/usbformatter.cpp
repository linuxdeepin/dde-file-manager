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

#include "usbformatter.h"
#include "dbusservice/dbusadaptor/usbformatter_adaptor.h"
#include "../partman/command.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QtConcurrent>
#include <QThreadPool>
#include <QFuture>
#include <QDebug>

QString UsbFormatter::ObjectPath = "/com/deepin/filemanager/daemon/UsbFormatter";


UsbFormatter::UsbFormatter(QObject *parent) : QObject(parent)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_usbFormatterAdaptor = new UsbFormatterAdaptor(this);
    m_partitionManager = new PartMan::PartitionManager(this);
}

bool UsbFormatter::mkfs(const QString &path, const QString &fs, const QString &label)
{
    typedef bool (PartMan::PartitionManager::*mkfs) (const QString &, const QString & ,const QString &);
    QFuture<bool> future = QtConcurrent::run(QThreadPool::globalInstance(), m_partitionManager,
                                             static_cast<mkfs>(&PartMan::PartitionManager::mkfs),
                                             path, fs, label);
    future.waitForFinished();
    bool ret = future.result();
    return ret;
}
