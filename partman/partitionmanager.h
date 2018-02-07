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

#ifndef PARTITIONMANAGER_H
#define PARTITIONMANAGER_H

#include <QObject>

namespace PartMan {

class Partition;

class PartitionManager : public QObject
{
    Q_OBJECT
public:

    enum FsType {
        Btrfs,
        Efi,
        Ext2,
        Ext3,
        Ext4,
        F2fs,
        Fat16,
        Fat32,
        Hfs,
        Hfsplus,
        Jfs,
        Linuxswap,
        Lvm2pv,
        Nilfs2,
        Ntfs,
        Reiser4,
        Reiserfs,
        Xfs,
        Unknown,
    };

    Q_ENUM(FsType)

    static int getMaxNameLengthByTypeString(const QString& typeStr);
    static int getMaxNameLengthByType(const FsType& type);

    explicit PartitionManager(QObject *parent = 0);

signals:

public slots:
    bool mkfs(const Partition& partition);
    bool mkfs(const QString& path, const QString& fs, const QString& label);

    bool actionFormatBtrfs(const Partition& partition);
    bool actionFormatBtrfs(const QString& path, const QString& label);

    bool actionFormatEfi(const Partition& partition);
    bool actionFormatEfi(const QString& path, const QString& label);

    bool actionFormatExt2(const Partition& partition);
    bool actionFormatExt2(const QString& path, const QString& label);

    bool actionFormatExt3(const Partition& partition);
    bool actionFormatExt3(const QString& path, const QString& label);

    bool actionFormatExt4(const Partition& partition);
    bool actionFormatExt4(const QString& path, const QString& label);

    bool actionFormatF2fs(const Partition& partition);
    bool actionFormatF2fs(const QString& path, const QString& label);

    bool actionFormatFat16(const Partition& partition);
    bool actionFormatFat16(const QString& path, const QString& label);

    bool actionFormatFat32(const Partition& partition);
    bool actionFormatFat32(const QString& path, const QString& label);

    bool actionFormatHfs(const Partition& partition);
    bool actionFormatHfs(const QString& path, const QString& label);

    bool actionFormatHfsplus(const Partition& partition);
    bool actionFormatHfsplus(const QString& path, const QString& label);

    bool actionFormatJfs(const Partition& partition);
    bool actionFormatJfs(const QString& path, const QString& label);

    bool actionFormatLinuxswap(const Partition& partition);
    bool actionFormatLinuxswap(const QString& path, const QString& label);

    bool actionFormatLvm2pv(const Partition& partition);
    bool actionFormatLvm2pv(const QString& path, const QString& label);

    bool actionFormatNilfs2(const Partition& partition);
    bool actionFormatNilfs2(const QString& path, const QString& label);

    bool actionFormatNtfs(const Partition& partition);
    bool actionFormatNtfs(const QString& path, const QString& label);

    bool actionFormatReiser4(const Partition& partition);
    bool actionFormatReiser4(const QString& path, const QString& label);

    bool actionFormatReiserfs(const Partition& partition);
    bool actionFormatReiserfs(const QString& path, const QString& label);

    bool actionFormatXfs(const Partition& partition);
    bool actionFormatXfs(const QString& path, const QString& label);

    bool actionFormatUnknown(const Partition& partition);
    bool actionFormatUnknown(const QString& path, const QString& label);
};

}
#endif // PARTITIONMANAGER_H
