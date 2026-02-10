// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMSETUP_H
#define DMSETUP_H

#include <QString>
#include <libdevmapper.h>

namespace dm_setup {
struct DMTable
{
    QString targetType;
    QString targetArgs;
    quint64 startSector;
    quint64 sectorCount;
};

int dmSetDeviceTable(const QString &dmDev, const DMTable &table, int taskType);
int dmCreateDevice(const QString &dmDev, const DMTable &table);
int dmSuspendDevice(const QString &dmDev);
int dmResumeDevice(const QString &dmDev);
int dmReloadDevice(const QString &dmDev, const DMTable &table);
int dmRemoveDevice(const QString &dmDev);
}   // namespace dm_setup

namespace dm_setup_helper {
QString findHolderDev(const QString &dev);

struct ProcPartition   // /proc/partitions item
{
    int major;
    int minor;
    quint64 blocks;
    QString name;
    QString devNum()
    {
        return QString("%1:%2").arg(major).arg(minor);
    }
};
QList<ProcPartition> procPartitions();
}   // namespace dm_setup_helper

#endif   // DMSETUP_H
