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

#ifndef READUSAGEMANAGER_H
#define READUSAGEMANAGER_H

#include <QObject>

namespace PartMan {

class ReadUsageManager : public QObject
{
    Q_OBJECT
public:
    explicit ReadUsageManager(QObject *parent = 0);

    static qlonglong ParseBtrfsUnit(const QString& value);

signals:

public slots:
    bool readUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readUsage(const QString& path, const QString& fs, qlonglong& freespace, qlonglong& total);
    bool readBtrfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readEfiUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readExt2Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readExt3Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readExt4Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readF2fsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readFat16Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readFat32Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readHfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readHfsplusUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readJfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readLinuxswapUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readLvm2pvUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readNilfs2Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readNtfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readReiser4Usage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readReiserfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readXfsUsage(const QString& path, qlonglong& freespace, qlonglong& total);
    bool readUnknownUsage(const QString& path, qlonglong& freespace, qlonglong& total);
};
}
#endif // READUSAGEMANAGER_H
