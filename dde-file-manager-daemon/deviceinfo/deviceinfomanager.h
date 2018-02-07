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

#ifndef DEVICEINFOMANAGER_H
#define DEVICEINFOMANAGER_H

#include <QObject>
#include "../partman/readusagemanager.h"
#include "../partman/partition.h"

class DeviceInfoManagerAdaptor;


class DeviceInfoManager : public QObject
{
    Q_OBJECT
public:
    static QString ObjectPath;
    explicit DeviceInfoManager(QObject *parent = 0);

signals:

public slots:
    PartMan::Partition getPartitionByDevicePath(const QString& devicePath);
    bool readUsage(const QString &path, qlonglong &freespace, qlonglong &total);

private:
    DeviceInfoManagerAdaptor* m_deviceInfoManagerAdaptor = NULL;
    PartMan::ReadUsageManager* m_readUsageManager = NULL;
};

#endif // DEVICEINFOMANAGER_H
