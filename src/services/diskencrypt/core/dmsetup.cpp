// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmsetup.h"

#include <QDebug>
#include <QFile>
#include <QDir>

class DMTaskHelper
{
public:
    DMTaskHelper(int taskType)
    {
        m_task = dm_task_create(taskType);
        if (!m_task)
            qWarning() << "cannot create task!" << taskType;
    }
    ~DMTaskHelper()
    {
        if (m_task)
            dm_task_destroy(m_task);
    }

    struct dm_task *task()
    {
        return m_task;
    }

private:
    struct dm_task *m_task { nullptr };
};

int dm_setup::dmSetDeviceTable(const QString &dmDev, const DMTable &table, int taskType)
{
    auto h = DMTaskHelper(taskType);
    if (!h.task())
        return -1;

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot set dm name!" << err << dmDev;
        return -err;
    }

    r = dm_task_add_target(h.task(),
                           table.startSector,
                           table.sectorCount,
                           table.targetType.toStdString().c_str(),
                           table.targetArgs.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot add target for device!" << dmDev << err;
        return -err;
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot run task!" << dmDev << err;
        return -err;
    }

    dm_task_update_nodes();
    return 0;
}

int dm_setup::dmCreateDevice(const QString &dmDev, const DMTable &table)
{
    return dmSetDeviceTable(dmDev, table, DM_DEVICE_CREATE);
}

int dm_setup::dmSuspendDevice(const QString &dmDev)
{
    auto h = DMTaskHelper(DM_DEVICE_SUSPEND);
    if (!h.task())
        return -1;

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot set dm name for suspend!" << err << dmDev;
        return -err;
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot run suspend task!" << err << dmDev;
        return -err;
    }
    return 0;
}

int dm_setup::dmResumeDevice(const QString &dmDev)
{
    auto h = DMTaskHelper(DM_DEVICE_RESUME);
    if (!h.task())
        return -1;

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot set dm name for resume!" << err << dmDev;
        return -err;
    }

    uint32_t cookie = 0;
    uint16_t udevFlags = DM_UDEV_DISABLE_LIBRARY_FALLBACK;
    if (dm_udev_get_sync_support()){
        r = dm_task_set_cookie(h.task(), &cookie, udevFlags);
        if (r == 0) {
            auto err = dm_task_get_errno(h.task());
            qWarning() << "cannot set cookie for resuming!" << err << r << dmDev;
            return -r;
        }
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot run resume task!" << err << dmDev;
        return -err;
    }

    if (cookie && dm_udev_get_sync_support())
        dm_udev_wait(cookie);

    dm_task_update_nodes();
    return 0;
}

int dm_setup::dmReloadDevice(const QString &dmDev, const DMTable &table)
{
    return dmSetDeviceTable(dmDev, table, DM_DEVICE_RELOAD);
}

QString dm_setup_helper::findHolderDev(const QString &dev)
{
    if (!dev.startsWith("/dev/dm-"))
        return dev;

    QString name = dev.mid(5);
    const auto partitions = procPartitions();
    for (auto p : partitions) {
        QString path("/sys/dev/block/%1/holders");
        QDir d(path.arg(p.devNum()));
        if (!d.exists()) continue;

        auto devs = d.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (!devs.contains(name))
            continue;
        if (p.name.startsWith("dm-"))
            return findHolderDev("/dev/" + p.name);
        else
            return "/dev/" + p.name;
    }

    return "";
}

QList<dm_setup_helper::ProcPartition> dm_setup_helper::procPartitions()
{
    QFile f("/proc/partitions");
    if (!f.open(QIODevice::ReadOnly))
        return {};
    QList<ProcPartition> partitions;
    auto contents = f.readAll();
    auto lines = contents.split('\n');
    for (auto line : lines) {
        auto fields = QString(line).split(' ', Qt::SkipEmptyParts);
        if (fields.count() < 4) continue;
        bool canInt = false;
        fields[0].toInt(&canInt);
        if (!canInt) continue;

        partitions.append({ fields[0].toInt(),
                            fields[1].toInt(),
                            fields[2].toULongLong(),
                            fields[3] });
    }
    f.close();
    return partitions;
}

int dm_setup::dmRemoveDevice(const QString &dmDev)
{
    auto h = DMTaskHelper(DM_DEVICE_REMOVE);
    if (!h.task())
        return -1;

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot set dm name for remove!" << err << dmDev;
        return -err;
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qWarning() << "cannot run remove task!" << err << dmDev;
        return -err;
    }

    dm_task_update_nodes();

    return 0;
}
