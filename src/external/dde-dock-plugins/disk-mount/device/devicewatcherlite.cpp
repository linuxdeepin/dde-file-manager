// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicewatcherlite.h"

#include <dfm-mount/ddevicemanager.h>
#include <dfm-mount/dblockmonitor.h>
#include <dfm-mount/dprotocolmonitor.h>
#include <dfm-mount/dblockdevice.h>
#include <dfm-mount/dprotocoldevice.h>

#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QThread>
#include <QPointer>
#include <QRegularExpression>

#include <libmount.h>

DFM_MOUNT_USE_NS

DeviceWatcherLite::DeviceWatcherLite(QObject *parent)
    : QObject(parent)
{
    DDeviceManager::instance();

    connect(DDeviceManager::instance(), &DDeviceManager::propertyChanged, this, [this](const QString &, const QMap<Property, QVariant> &changes, DeviceType type) {
        if (type == DeviceType::kBlockDevice && changes.contains(Property::kBlockHintIgnore))
            Q_EMIT hintIgnoreChanged();
    });
    connect(DDeviceManager::instance(), &DDeviceManager::mounted, this, [this](const QString &id, const QString &mpt, DeviceType type) {
        if (type == DeviceType::kBlockDevice)
            Q_EMIT this->blockDeviceMounted(id, mpt);
        else if (type == DeviceType::kProtocolDevice)
            Q_EMIT this->protocolDeviceMounted(id, mpt);
    });
    connect(DDeviceManager::instance(), &DDeviceManager::unmounted, this, [this](const QString &id, DeviceType type) {
        if (type == DeviceType::kBlockDevice)
            Q_EMIT this->blockDeviceUnmounted(id);
        else if (type == DeviceType::kProtocolDevice)
            Q_EMIT this->protocolDeviceUnmounted(id);
    });

    auto blkMonitor = DDeviceManager::instance()->getRegisteredMonitor(DeviceType::kBlockDevice).objectCast<DBlockMonitor>();
    connect(blkMonitor.data(), &DBlockMonitor::fileSystemAdded, this, &DeviceWatcherLite::blockFileSystemAdded);
    connect(blkMonitor.data(), &DBlockMonitor::fileSystemRemoved, this, &DeviceWatcherLite::blockFileSystemRemoved);
    connect(blkMonitor.data(), &DBlockMonitor::driveAdded, this, &DeviceWatcherLite::blockDriveAdded);
    connect(blkMonitor.data(), &DBlockMonitor::driveRemoved, this, &DeviceWatcherLite::blockDriveRemoved);

    DDeviceManager::instance()->startMonitorWatch();
}

DeviceWatcherLite *DeviceWatcherLite::instance()
{
    static DeviceWatcherLite ins;
    return &ins;
}

QStringList DeviceWatcherLite::allMountedRemovableBlocks()
{
    const QStringList &devs = DDeviceManager::instance()->devices(DeviceType::kBlockDevice).value(DeviceType::kBlockDevice, {});
    auto monitor = DDeviceManager::instance()->getRegisteredMonitor(DeviceType::kBlockDevice);
    Q_ASSERT(monitor);

    blksOfDrv.clear();
    QStringList mountedRemovable;
    for (const auto &dev : devs) {
        QSharedPointer<DBlockDevice> devPtr = monitor->createDeviceById(dev).objectCast<DBlockDevice>();
        if (!devPtr)
            continue;
        if (!devPtr->removable())
            continue;

        QString encryptedMpt;
        if (devPtr->isEncrypted()) {
            QString clearDevID = devPtr->getProperty(Property::kEncryptedCleartextDevice).toString();
            if (clearDevID.isEmpty()) {
                continue;
            } else {
                QSharedPointer<DBlockDevice> clearDev = monitor->createDeviceById(clearDevID).objectCast<DBlockDevice>();
                if (!clearDev || clearDev->mountPoint().isEmpty())
                    continue;
                encryptedMpt = clearDev->mountPoint();
            }
        }

        // NOTE(xust): removable/hintSystem is not always correct in some certain hardwares.
        if (!devPtr->canPowerOff() && !devPtr->optical())
            continue;
        // ignore blocks not mounted under /media/
        if (!devPtr->mountPoint().startsWith("/media/")
            && !encryptedMpt.startsWith("/media"))
            continue;

        if (isSiblingOfRoot(devPtr))
            continue;

        mountedRemovable.append(dev);
        blksOfDrv[devPtr->drive()].append(dev);
    }

    return mountedRemovable;
}

QStringList DeviceWatcherLite::allMountedProtocols()
{
    const QStringList &devs = DDeviceManager::instance()->devices(DeviceType::kProtocolDevice).value(DeviceType::kProtocolDevice, {});
    return devs;
}

QSharedPointer<DBlockDevice> DeviceWatcherLite::createBlockDevicePtr(const QString &id)
{
    auto monitor = DDeviceManager::instance()->getRegisteredMonitor(DeviceType::kBlockDevice);
    Q_ASSERT(monitor);

    return monitor->createDeviceById(id).objectCast<DBlockDevice>();
}

QSharedPointer<DProtocolDevice> DeviceWatcherLite::createProtocolDevicePtr(const QString &id)
{
    auto monitor = DDeviceManager::instance()->getRegisteredMonitor(DeviceType::kProtocolDevice);
    Q_ASSERT(monitor);

    return monitor->createDeviceById(id).objectCast<DProtocolDevice>();
}

void DeviceWatcherLite::detachBlockDevice(const QString &id)
{
    auto blkPtr = createBlockDevicePtr(id);
    if (!blkPtr)
        return;

    const QString &drv = blkPtr->drive();
    QStringList siblings = blksOfDrv.value(drv, QStringList());

    QString clearDevID = blkPtr->getProperty(Property::kEncryptedCleartextDevice).toString();
    if (blkPtr->isEncrypted() && !clearDevID.isEmpty())
        siblings.append(clearDevID);

    QSharedPointer<bool> unmountDone(new bool(true));
    QSharedPointer<int> unmountCount(new int(siblings.count()));
    for (const auto &dev : siblings) {
        auto devPtr = createBlockDevicePtr(dev);
        if (!devPtr) continue;
        const QString &mpt = devPtr->mountPoint();
        unmountStacked(mpt);

        devPtr->unmountAsync({}, [=](bool ok, const OperationErrorInfo &) {
            *unmountDone &= ok;
            *unmountCount -= 1;
            if (*unmountCount == 0)
                removeDevice(*unmountDone, blkPtr);
        });
    }
}

void DeviceWatcherLite::detachProtocolDevice(const QString &id)
{
    auto proPtr = createProtocolDevicePtr(id);
    if (!proPtr)
        return;

    QPointer<DeviceWatcherLite> that(this);
    proPtr->unmountAsync({}, [=](bool ok, const OperationErrorInfo &err) {
        if (!ok && that)
            Q_EMIT this->operationFailed(kUnmount);
        qDebug() << "[disk-mount]: detach protocol device: " << id << err.message << err.code;
    });
}

void DeviceWatcherLite::detachAllDevices()
{
    allMountedRemovableBlocks();   // used to update the blksOfDev
    QStringList blks;
    std::for_each(blksOfDrv.cbegin(), blksOfDrv.cend(), [&](const QStringList &siblings) {
        if (siblings.count() > 0)
            blks.append(siblings.first());
    });

    qDebug() << "[disk-mount]: about to unmount blocks and its siblings: " << blks;
    std::for_each(blks.cbegin(), blks.cend(), [=](const QString &blk) { detachBlockDevice(blk); });

    const QStringList &protocols = allMountedProtocols();
    QStringList waitToUnmount;
    std::for_each(protocols.cbegin(), protocols.cend(), [&](const QString &proto) {
        if (!proto.startsWith("file") || proto.contains(QRegularExpression(R"(^file:///media/.*/smbmounts/)")))
            waitToUnmount.append(proto);
    });

    qDebug() << "[disk-mount]: about to unmount protocols: " << waitToUnmount;
    std::for_each(waitToUnmount.cbegin(), waitToUnmount.cend(), [this](const QString &proto) { detachProtocolDevice(proto); });
}

void DeviceWatcherLite::unmountStacked(const QString &mpt)
{
    if (mpt.isEmpty())
        return;

    static constexpr char kDaemonService[] { "com.deepin.filemanager.daemon" };
    static constexpr char kDaemonMountPath[] { "/com/deepin/filemanager/daemon/MountControl" };
    static constexpr char kDaemonMountIface[] { "com.deepin.filemanager.daemon.MountControl" };

    qDebug() << "[disk-mount]: construct daemon interface";
    QDBusInterface iface(kDaemonService, kDaemonMountPath, kDaemonMountIface, QDBusConnection::systemBus());
    qDebug() << "[disk-mount]: constructed daemon interface";
    iface.setTimeout(1000);
    QDBusReply<QVariantMap> reply = iface.call("Unmount", mpt,
                                               QVariantMap { { "fsType", "common" }, { "unmountAllStacked", true } });
    const auto &ret = reply.value();
    qDebug() << "unmount all stacked mount of: " << mpt << ret;
}

void DeviceWatcherLite::removeDevice(bool unmountDone, QSharedPointer<dfmmount::DBlockDevice> blk)
{
    if (!blk)
        return;

    if (!unmountDone) {
        Q_EMIT this->operationFailed(kUnmount);
        return;
    }

    QThread::msleep(500);
    QPointer<DeviceWatcherLite> that(this);

    auto doPowerOff = [=] {
        blk->powerOffAsync({}, [=](bool ok, const OperationErrorInfo &err) {
            if (that && !ok)
                Q_EMIT this->operationFailed(kPowerOff);
            qDebug() << "[disk-mount]: poweroff device: " << err.message << err.code;
        });
    };

    if (blk->optical()) {
        blk->ejectAsync({}, [=](bool ok, const OperationErrorInfo &err) {
            if (that && !ok)
                Q_EMIT this->operationFailed(kEject);
            qDebug() << "[disk-mount]: eject device: " << err.message << err.code;
        });
    } else {
        if (blk->isEncrypted()) {
            blk->lockAsync({}, [=](bool ok, const OperationErrorInfo &) {
                if (ok)
                    doPowerOff();
                else if (that)
                    Q_EMIT this->operationFailed(kPowerOff);
            });
        } else {
            doPowerOff();
        }
    }
}

bool DeviceWatcherLite::isSiblingOfRoot(QSharedPointer<DBlockDevice> blkDev)
{
    static QString rootDrive;
    static std::once_flag flg;
    std::call_once(flg, [&rootDrive, this] {
        const QString &rootDev = getMountInfo("/", kSearchByDevice);
        const QString &rootDevId = "/org/freedesktop/UDisks2/block_devices/" + rootDev.mid(5);
        QSharedPointer<DBlockDevice> rootBlk = createBlockDevicePtr(rootDevId);
        rootDrive = rootBlk ? rootBlk->drive() : "";
        qInfo() << "got root drive:" << rootDrive << rootDev;
    });

    return rootDrive == blkDev->drive();
}

QString DeviceWatcherLite::getMountInfo(const QString &in, SearchBy what)
{
    libmnt_table *tab { mnt_new_table() };
    if (!tab)
        return {};

    if (mnt_table_parse_mtab(tab, nullptr) != 0) {
        qWarning() << "Invalid mnt_table_parse_mtab call";
        if (tab) mnt_free_table(tab);
        return {};
    }

    auto query = (what == kSearchByMountPoint) ? mnt_table_find_source : mnt_table_find_target;
    auto get = (what == kSearchByMountPoint) ? mnt_fs_get_target : mnt_fs_get_source;
    std::string stdPath { in.toStdString() };
    auto fs = query(tab, stdPath.c_str(), MNT_ITER_BACKWARD);
    if (fs) {
        QString out = get(fs);
        if (tab) mnt_free_table(tab);
        return out;
    }

    qWarning() << "Invalid libmnt_fs*";
    if (tab) mnt_free_table(tab);
    return {};
}
