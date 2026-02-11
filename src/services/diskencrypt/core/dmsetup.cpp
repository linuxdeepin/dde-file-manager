// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
            qCritical() << "[DMTaskHelper] Failed to create DM task for type:" << taskType;
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
    qInfo() << "[dm_setup::dmSetDeviceTable] Setting device table for:" << dmDev << "task type:" << taskType;
    
    auto h = DMTaskHelper(taskType);
    if (!h.task()) {
        qCritical() << "[dm_setup::dmSetDeviceTable] Failed to create DM task helper for device:" << dmDev;
        return -1;
    }

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmSetDeviceTable] Failed to set DM device name:" << dmDev << "error:" << err;
        return -err;
    }

    r = dm_task_add_target(h.task(),
                           table.startSector,
                           table.sectorCount,
                           table.targetType.toStdString().c_str(),
                           table.targetArgs.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmSetDeviceTable] Failed to add target for device:" << dmDev << "error:" << err << "target type:" << table.targetType;
        return -err;
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmSetDeviceTable] Failed to run DM task for device:" << dmDev << "error:" << err;
        return -err;
    }

    dm_task_update_nodes();
    qInfo() << "[dm_setup::dmSetDeviceTable] Successfully set device table for:" << dmDev;
    return 0;
}

int dm_setup::dmCreateDevice(const QString &dmDev, const DMTable &table)
{
    return dmSetDeviceTable(dmDev, table, DM_DEVICE_CREATE);
}

int dm_setup::dmSuspendDevice(const QString &dmDev)
{
    qInfo() << "[dm_setup::dmSuspendDevice] Suspending DM device:" << dmDev;
    
    auto h = DMTaskHelper(DM_DEVICE_SUSPEND);
    if (!h.task()) {
        qCritical() << "[dm_setup::dmSuspendDevice] Failed to create DM task helper for device:" << dmDev;
        return -1;
    }

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmSuspendDevice] Failed to set DM device name for suspend:" << dmDev << "error:" << err;
        return -err;
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmSuspendDevice] Failed to run suspend task for device:" << dmDev << "error:" << err;
        return -err;
    }
    
    qInfo() << "[dm_setup::dmSuspendDevice] Successfully suspended DM device:" << dmDev;
    return 0;
}

int dm_setup::dmResumeDevice(const QString &dmDev)
{
    qInfo() << "[dm_setup::dmResumeDevice] Resuming DM device:" << dmDev;
    
    auto h = DMTaskHelper(DM_DEVICE_RESUME);
    if (!h.task()) {
        qCritical() << "[dm_setup::dmResumeDevice] Failed to create DM task helper for device:" << dmDev;
        return -1;
    }

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmResumeDevice] Failed to set DM device name for resume:" << dmDev << "error:" << err;
        return -err;
    }

    uint32_t cookie = 0;
    uint16_t udevFlags = DM_UDEV_DISABLE_LIBRARY_FALLBACK;
    if (dm_udev_get_sync_support()){
        r = dm_task_set_cookie(h.task(), &cookie, udevFlags);
        if (r == 0) {
            auto err = dm_task_get_errno(h.task());
            qWarning() << "[dm_setup::dmResumeDevice] Failed to set cookie for resuming device:" << dmDev << "error:" << err;
            return -r;
        }
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmResumeDevice] Failed to run resume task for device:" << dmDev << "error:" << err;
        return -err;
    }

    if (cookie && dm_udev_get_sync_support())
        dm_udev_wait(cookie);

    dm_task_update_nodes();
    qInfo() << "[dm_setup::dmResumeDevice] Successfully resumed DM device:" << dmDev;
    return 0;
}

int dm_setup::dmReloadDevice(const QString &dmDev, const DMTable &table)
{
    return dmSetDeviceTable(dmDev, table, DM_DEVICE_RELOAD);
}

QString dm_setup_helper::findHolderDev(const QString &dev)
{
    qInfo() << "[dm_setup_helper::findHolderDev] Finding holder device for:" << dev;
    
    if (!dev.startsWith("/dev/dm-")) {
        qInfo() << "[dm_setup_helper::findHolderDev] Device is not a DM device, returning as-is:" << dev;
        return dev;
    }

    QString name = dev.mid(5);
    const auto partitions = procPartitions();
    for (auto p : partitions) {
        QString path("/sys/dev/block/%1/holders");
        QDir d(path.arg(p.devNum()));
        if (!d.exists()) continue;

        auto devs = d.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (!devs.contains(name))
            continue;
        if (p.name.startsWith("dm-")) {
            qInfo() << "[dm_setup_helper::findHolderDev] Found nested DM device, recursing for:" << p.name;
            return findHolderDev("/dev/" + p.name);
        } else {
            qInfo() << "[dm_setup_helper::findHolderDev] Found holder device:" << p.name << "for device:" << dev;
            return "/dev/" + p.name;
        }
    }

    qWarning() << "[dm_setup_helper::findHolderDev] No holder device found for:" << dev;
    return "";
}

QList<dm_setup_helper::ProcPartition> dm_setup_helper::procPartitions()
{
    qDebug() << "[dm_setup_helper::procPartitions] Reading partition information from /proc/partitions";
    
    QFile f("/proc/partitions");
    if (!f.open(QIODevice::ReadOnly)) {
        qCritical() << "[dm_setup_helper::procPartitions] Failed to open /proc/partitions";
        return {};
    }
    
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
    qDebug() << "[dm_setup_helper::procPartitions] Found" << partitions.size() << "partitions";
    return partitions;
}

int dm_setup::dmRemoveDevice(const QString &dmDev)
{
    qInfo() << "[dm_setup::dmRemoveDevice] Removing DM device:" << dmDev;
    
    auto h = DMTaskHelper(DM_DEVICE_REMOVE);
    if (!h.task()) {
        qCritical() << "[dm_setup::dmRemoveDevice] Failed to create DM task helper for device:" << dmDev;
        return -1;
    }

    int r = dm_task_set_name(h.task(), dmDev.toStdString().c_str());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmRemoveDevice] Failed to set DM device name for removal:" << dmDev << "error:" << err;
        return -err;
    }

    r = dm_task_run(h.task());
    if (r == 0) {
        auto err = dm_task_get_errno(h.task());
        qCritical() << "[dm_setup::dmRemoveDevice] Failed to run remove task for device:" << dmDev << "error:" << err;
        return -err;
    }

    dm_task_update_nodes();

    return 0;
}
