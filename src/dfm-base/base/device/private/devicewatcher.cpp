// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicewatcher.h"
#include "devicewatcher_p.h"
#include "devicehelper.h"
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/finallyutil.h>

#include <QVariantMap>
#include <QDebug>
#include <QStorageInfo>
#include <QtConcurrent>

#include <dfm-mount/dmount.h>
#include <dfm-burn/dopticaldiscinfo.h>
#include <dfm-burn/dopticaldiscmanager.h>

#include <sys/statvfs.h>

using namespace dfmbase;
DFM_MOUNT_USE_NS
using namespace GlobalServerDefines;

DeviceWatcher::DeviceWatcher(QObject *parent)
    : QObject(parent), d(new DeviceWatcherPrivate(this))
{
    // 在构造函数中一次性连接定时器信号，避免重复连接
    connect(&d->pollingTimer, &QTimer::timeout, d.data(), &DeviceWatcherPrivate::queryUsageAsync);
}

DeviceWatcher::~DeviceWatcher()
{
}

void DeviceWatcher::startPollingUsage()
{
    if (d->pollingTimer.isActive())
        return;

    qCInfo(logDFMBase) << "Starting device usage polling";
    d->pollingTimer.start(d->kPollingInterval);   // 然后启动定时器
}

void DeviceWatcher::stopPollingUsage()
{
    if (!d->pollingTimer.isActive())
        return;

    qCInfo(logDFMBase) << "Stopping device usage polling";
    d->pollingTimer.stop();
}

void DeviceWatcher::initUsageCache()
{
    // 只在定时器未运行时执行，避免重复查询
    if (!d->pollingTimer.isActive()) {
        qCInfo(logDFMBase) << "Initializing device usage cache (one-time query, no timer)";
        d->queryUsageAsync();
    }
}

void DeviceWatcher::refreshUsage()
{
    qCInfo(logDFMBase) << "Refreshing device usage on client request";
    d->queryUsageAsync();
}

void DeviceWatcherPrivate::queryUsageAsync()
{
    qCInfo(logDFMBase) << "Query device usage";
    QThreadPool::globalInstance()->start([this] {
        auto blocks = allBlockInfos;
        auto protocols = allProtocolInfos;
        std::for_each(blocks.cbegin(), blocks.cend(),
                      [this](const QVariantMap &item) { queryUsageOfItem(item, DeviceType::kBlockDevice); });
        std::for_each(protocols.cbegin(), protocols.cend(),
                      [this](const QVariantMap &item) { queryUsageOfItem(item, DeviceType::kProtocolDevice); });
    });
}

void DeviceWatcherPrivate::updateStorage(const QString &id, quint64 total, quint64 avai)
{
    auto update = [&](QHash<QString, QVariantMap> &container) {
        if (!container.contains(id))
            return;
        qint64 used = static_cast<qint64>(total - avai);
        used = used < 0 ? 0 : used;

        auto &data = container[id];
        data[DeviceProperty::kSizeTotal] = total;
        data[DeviceProperty::kSizeUsed] = static_cast<quint64>(used);
        data[DeviceProperty::kSizeFree] = avai;
    };

    if (id.startsWith(kBlockDeviceIdPrefix))
        update(allBlockInfos);
    else
        update(allProtocolInfos);
}

void DeviceWatcherPrivate::queryUsageOfItem(const QVariantMap &itemData, dfmmount::DeviceType type)
{
    const QString &mpt = itemData.value(DeviceProperty::kMountPoint).toString();
    if (mpt.isEmpty())
        return;

    DevStorage old { itemData.value(DeviceProperty::kSizeTotal).toULongLong(),
                     itemData.value(DeviceProperty::kSizeFree).toULongLong(),
                     itemData.value(DeviceProperty::kSizeUsed).toULongLong() };

    if (type == DFMMOUNT::DeviceType::kAllDevice)
        return;

    DevStorage newStorage = (type == dfmmount::DeviceType::kBlockDevice)
            ? queryUsageOfBlock(itemData)
            : queryUsageOfProtocol(itemData);

    if (/*old != newStorage && */ newStorage.isValid()) {
        const QString &devId = itemData.value(DeviceProperty::kId).toString();
        emit DevMngIns->devSizeChanged(devId,
                                       newStorage.total,
                                       newStorage.avai);
    }
}

DevStorage DeviceWatcherPrivate::queryUsageOfBlock(const QVariantMap &itemData)
{
    DevStorage storage;
    if (!DeviceHelper::queryUsageOfBlockRealTime(itemData, &storage.total, &storage.avai, &storage.used))
        return {};

    return storage;
}

DevStorage DeviceWatcherPrivate::queryUsageOfProtocol(const QVariantMap &itemData)
{
    DevStorage storage;
    if (!DeviceHelper::queryUsageOfProtocolRealTime(itemData, &storage.total, &storage.avai, &storage.used))
        return {};

    return storage;
}

void DeviceWatcher::initDevDatas()
{
    qCInfo(logDFMBase) << "Device data initialization started";
    auto mng { DDeviceManager::instance() };
    const auto &devs { mng->devices() };
    for (const auto &dev : devs.value(DeviceType::kBlockDevice))
        d->allBlockInfos.insert(dev, DeviceHelper::loadBlockInfo(dev));
    for (const auto &dev : devs.value(DeviceType::kProtocolDevice))
        d->allProtocolInfos.insert(dev, DeviceHelper::loadProtocolInfo(dev));
    qCInfo(logDFMBase) << "Device data initialization completed - block devices:"
                       << d->allBlockInfos.size() << "protocol devices:" << d->allProtocolInfos.size();
}

/*!
 * \brief DeviceWatcher::queryOpticalDevUsage
 * \param id
 * query usage of optical item is a little complicated, first query info from libdfm-burn before mount,
 * and the datas will be saved to a local file, and it's indexed by device descriptor like 'sr0'.
 * if launch dfm with desktop running, when optical item mounted, the usage data is readed from local
 * files and a signal is emitted from desktop, dfm will receive it and update the display.
 * see 'DeviceHelper::persistentOpticalInfo and DeviceHelper::readOpticalInfo' functions.
 */
void DeviceWatcher::queryOpticalDevUsage(const QString &id)
{
    FinallyUtil final([id] { qCDebug(logDFMBase) << "Optical device usage query completed for device:" << id; });
    Q_UNUSED(final);

    QVariantMap data = DeviceHelper::loadBlockInfo(id);
    if (data.value(DeviceProperty::kId).toString().isEmpty()) {
        qCWarning(logDFMBase) << "Failed to load block info for optical device:" << id;
        return;
    }
    bool sizeChanged = false;
    QScopedPointer<DFMBURN::DOpticalDiscInfo> info { DFMBURN::DOpticalDiscManager::createOpticalInfo(data.value(DeviceProperty::kDevice).toString()) };
    if (info) {
        sizeChanged = true;
        data[DeviceProperty::kSizeTotal] = static_cast<quint64>(info->totalSize());
        data[DeviceProperty::kSizeUsed] = static_cast<quint64>(info->usedSize());
        data[DeviceProperty::kSizeFree] = static_cast<quint64>(info->totalSize() - info->usedSize());
        data[DeviceProperty::kOpticalMediaType] = static_cast<int>(info->mediaType());
        data[DeviceProperty::kOpticalWriteSpeed] = info->writeSpeed();
    }
    if (sizeChanged)
        saveOpticalDevUsage(id, data);
}

/*!
 * \brief DeviceWatcher::updateOpticalDevUsage
 * \param id
 * \param mpt
 *
 * TODO(zhangs): this is WORKAROUND!!!
 * refactor: optical disc info by SCSI
 */
void DeviceWatcher::updateOpticalDevUsage(const QString &id, const QString &mpt)
{
    FinallyUtil final([id] { qCDebug(logDFMBase) << "Optical device usage update completed for device:" << id; });
    Q_UNUSED(final);
    if (mpt.isEmpty()) {
        qCWarning(logDFMBase) << "Mount point is empty for optical device:" << id;
        return;
    }

    QVariantMap data = DeviceHelper::loadBlockInfo(id);
    if (data.value(DeviceProperty::kId).toString().isEmpty() || !data.value(DeviceProperty::kOptical).toBool()) {
        qCWarning(logDFMBase) << "Invalid optical device data for device:" << id;
        return;
    }

    const QString &mediaType { DeviceUtils::formatOpticalMediaType(data.value(DeviceProperty::kMedia).toString()) };
    if (mediaType != "DVD+RW" && mediaType != "DVD-RW") {
        qCDebug(logDFMBase) << "Optical device media type not supported for usage update - device:" << id << "type:" << mediaType;
        return;
    }

    const QString &fs { data.value(DeviceProperty::kFileSystem).toString() };
    if (fs != "udf") {
        qCDebug(logDFMBase) << "Optical device filesystem not supported for usage update - device:" << id << "filesystem:" << fs;
        return;
    }

    const quint64 freeSize { data.value(DeviceProperty::kSizeFree).toULongLong() };
    if (freeSize != 0) {
        qCDebug(logDFMBase) << "Optical device has free space, no usage update needed - device:" << id << "free size:" << freeSize;
        return;
    }

    // Note: QStorageInfo::bytesTotal is not accurate for DVD-RW
    // TODO(zhangs): For the total capacity of a DVD-RW disc,
    // the current total capacity is only the capacity of the region being **formatted**,
    // not the capacity of the physical disc. Although you can refer to the dvd+rw-mediainfo
    // implementation, this involves a lot of low level code and is not suitable to be written in dde-file-manager,
    // it should be implemented in `dfm-burn` in the future!
    QStorageInfo si(mpt);
    qint64 avai = si.bytesAvailable() > 0 ? si.bytesAvailable() : 0;
    data[DeviceProperty::kSizeUsed] = static_cast<quint64>(si.bytesTotal() - avai);

    qCDebug(logDFMBase) << "Optical device usage updated - device:" << id
                        << "total:" << si.bytesTotal() << "available:" << avai;
    saveOpticalDevUsage(id, data);
}

void DeviceWatcher::saveOpticalDevUsage(const QString &id, const QVariantMap &data)
{
    DeviceHelper::persistentOpticalInfo(data);
    emit DevMngIns->devSizeChanged(id, data[DeviceProperty::kSizeTotal].toULongLong(), data[DeviceProperty::kSizeFree].toULongLong());
    DevProxyMng->reloadOpticalInfo(id);
}

void DeviceWatcher::startWatch()
{
    if (d->isWatching) {
        qCInfo(logDFMBase) << "Device watcher is already monitoring device changes";
        return;
    }

    auto mng = DDeviceManager::instance();
    mng->startMonitorWatch();
    auto blkMonitor = mng->getRegisteredMonitor(DeviceType::kBlockDevice).objectCast<DBlockMonitor>();
    if (!blkMonitor) {
        qCCritical(logDFMBase) << "Failed to get block device monitor - device watching cannot start";
    } else {
        auto ptr = blkMonitor.data();
        d->connections << connect(ptr, &DBlockMonitor::driveAdded, DeviceManager::instance(), &DeviceManager::blockDriveAdded);
        d->connections << connect(ptr, &DBlockMonitor::driveRemoved, DeviceManager::instance(), &DeviceManager::blockDriveRemoved);
        d->connections << connect(ptr, &DBlockMonitor::deviceAdded, this, &DeviceWatcher::onBlkDevAdded);
        d->connections << connect(ptr, &DBlockMonitor::deviceRemoved, this, &DeviceWatcher::onBlkDevRemoved);
        d->connections << connect(ptr, &DBlockMonitor::mountAdded, this, &DeviceWatcher::onBlkDevMounted);
        d->connections << connect(ptr, &DBlockMonitor::mountRemoved, this, &DeviceWatcher::onBlkDevUnmounted);
        d->connections << connect(ptr, &DBlockMonitor::blockLocked, this, &DeviceWatcher::onBlkDevLocked);
        d->connections << connect(ptr, &DBlockMonitor::blockUnlocked, this, &DeviceWatcher::onBlkDevUnlocked);
        d->connections << connect(ptr, &DBlockMonitor::fileSystemAdded, this, &DeviceWatcher::onBlkDevFsAdded);
        d->connections << connect(ptr, &DBlockMonitor::fileSystemRemoved, this, &DeviceWatcher::onBlkDevFsRemoved);
        d->connections << connect(ptr, &DBlockMonitor::propertyChanged, this, &DeviceWatcher::onBlkDevPropertiesChanged);

        d->isWatching = true;
        qCInfo(logDFMBase) << "Block device monitor connected successfully";
    }

    auto protoMonitor = mng->getRegisteredMonitor(DeviceType::kProtocolDevice).objectCast<DProtocolMonitor>();
    if (!protoMonitor) {
        qCCritical(logDFMBase) << "Failed to get protocol device monitor - protocol device watching cannot start";
    } else {
        auto ptr = protoMonitor.data();
        d->connections << connect(ptr, &DProtocolMonitor::deviceAdded, this, &DeviceWatcher::onProtoDevAdded);
        d->connections << connect(ptr, &DProtocolMonitor::deviceRemoved, this, &DeviceWatcher::onProtoDevRemoved);
        d->connections << connect(ptr, &DProtocolMonitor::mountAdded, this, &DeviceWatcher::onProtoDevMounted);
        d->connections << connect(ptr, &DProtocolMonitor::mountRemoved, this, &DeviceWatcher::onProtoDevUnmounted);

        d->isWatching = true;
        qCInfo(logDFMBase) << "Protocol device monitor connected successfully";
    }
}

void DeviceWatcher::stopWatch()
{
    for (const auto &conn : d->connections)
        disconnect(conn);
    d->connections.clear();
    d->isWatching = false;
    DDeviceManager::instance()->stopMonitorWatch();
}

void DeviceWatcher::onBlkDevAdded(const QString &id)
{
    qCInfo(logDFMBase) << "Block device added:" << id;
    auto dev = DeviceHelper::createBlockDevice(id);
    d->allBlockInfos.insert(id, DeviceHelper::loadBlockInfo(dev));

    emit DevMngIns->blockDevAdded(id);
    DevMngIns->doAutoMount(id, DeviceType::kBlockDevice);
}

void DeviceWatcher::onBlkDevRemoved(const QString &id)
{
    qCInfo(logDFMBase) << "Block device removed:" << id;
    QString oldMpt = d->allBlockInfos.value(id).value(DeviceProperty::kMountPoint).toString();
    d->allBlockInfos.remove(id);
    emit DevMngIns->blockDevRemoved(id, oldMpt);
}

void DeviceWatcher::onBlkDevMounted(const QString &id, const QString &mpt)
{
    const QVariantMap &info = d->allBlockInfos.value(id);
    // query info async avoid blocking main thread when disks' IO load is too high.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QtConcurrent::run(&DeviceWatcherPrivate::queryUsageOfItem, d.data(), info, DFMMOUNT::DeviceType::kBlockDevice);
#else
    QtConcurrent::run(d.data(), &DeviceWatcherPrivate::queryUsageOfItem, info, DFMMOUNT::DeviceType::kBlockDevice);
#endif
    emit DevMngIns->blockDevMounted(id, mpt);
}

void DeviceWatcher::onBlkDevUnmounted(const QString &id)
{
    QString oldMpt = d->allBlockInfos.value(id).value(DeviceProperty::kMountPoint).toString();
    d->allBlockInfos[id][DeviceProperty::kMountPoint] = QString();
    d->allBlockInfos[id].remove(DeviceProperty::kSizeFree);
    d->allBlockInfos[id].remove(DeviceProperty::kSizeUsed);
    emit DevMngIns->blockDevUnmounted(id, oldMpt);
}

void DeviceWatcher::onBlkDevLocked(const QString &id)
{
    emit DevMngIns->blockDevLocked(id);
}

void DeviceWatcher::onBlkDevUnlocked(const QString &id, const QString &cleartextId)
{
    emit DevMngIns->blockDevUnlocked(id, cleartextId);
}

void DeviceWatcher::onBlkDevFsAdded(const QString &id)
{
    // when a filesystem interface is added, lots of property may changed, so just reload the data
    // this will not happen frequently
    auto data = DeviceHelper::loadBlockInfo(id);
    d->allBlockInfos.insert(id, data);

    emit DevMngIns->blockDevFsAdded(id);
    emit DevMngIns->blockDevPropertyChanged(id, DeviceProperty::kHasFileSystem, true);

    auto mpt = data.value(DeviceProperty::kMountPoint).toString();
    if (!mpt.isEmpty())
        emit DevMngIns->blockDevMounted(id, mpt);
    else
        DevMngIns->doAutoMount(id, DeviceType::kBlockDevice);
}

void DeviceWatcher::onBlkDevFsRemoved(const QString &id)
{
    const auto oldData = d->allBlockInfos.value(id);
    auto data = DeviceHelper::loadBlockInfo(id);
    // when a filesystem interface is added, lots of property may changed, so just reload the data
    // this will not happen frequently
    if (!data.isEmpty())
        d->allBlockInfos.insert(id, data);
    else
        d->allBlockInfos.remove(id);

    auto oldMpt = oldData.value(DeviceProperty::kMountPoint).toString();
    if (!oldMpt.isEmpty())
        emit DevMngIns->blockDevUnmounted(id, oldMpt);
    emit DevMngIns->blockDevFsRemoved(id);
    emit DevMngIns->blockDevPropertyChanged(id, DeviceProperty::kHasFileSystem, false);
}

void DeviceWatcher::onBlkDevPropertiesChanged(const QString &id, const QMap<Property, QVariant> &changes)
{
    for (auto iter = changes.cbegin(); iter != changes.cend(); ++iter) {
        const auto &property = iter.key();
        const auto &var = iter.value();
        auto name = DeviceHelper::castFromDFMMountProperty(property);
        if (name.isEmpty()) {
            qCDebug(logDFMBase) << "Property has no mapped device name - property:" << Utils::getNameByProperty(property) << "value:" << var;
            continue;
        } else {
            QVariantMap &item = d->allBlockInfos[id];
            item[name] = var;

            // NOTE: when device's mountpoints changed to empty, do not update the cache immediatly, update it in blockDeviceUnmounted function
            // to report the unmounted path to subscribers.
            // Why? see TabBar:closeTabAndRemovecachedMnts
            if (name == DeviceProperty::kMountPoints)
                item[DeviceProperty::kMountPoint] = var.toStringList().isEmpty() ? item[DeviceProperty::kMountPoint] : var.toStringList().first();
            if (name == DeviceProperty::kOptical && !var.toBool()) {
                item[DeviceProperty::kOpticalMediaType] = "";
                item[DeviceProperty::kOpticalWriteSpeed] = QStringList();
                item[DeviceProperty::kSizeTotal] = 0;
                item[DeviceProperty::kSizeFree] = 0;
                item[DeviceProperty::kSizeUsed] = 0;
                DeviceHelper::persistentOpticalInfo(item);
                qCDebug(logDFMBase) << "Optical device properties cleared for device:" << id;
            }
            emit DevMngIns->blockDevPropertyChanged(id, name, var);
        }
    }
}

void DeviceWatcher::onProtoDevAdded(const QString &id)
{
    qCInfo(logDFMBase) << "Protocol device added:" << id;
    d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));

    emit DevMngIns->protocolDevAdded(id);
    DevMngIns->doAutoMount(id, DeviceType::kProtocolDevice);
}

void DeviceWatcher::onProtoDevRemoved(const QString &id)
{
    qCInfo(logDFMBase) << "Protocol device removed:" << id;
    QString oldMpt = d->allProtocolInfos.value(id).value(DeviceProperty::kMountPoint).toString();
    d->allProtocolInfos.remove(id);

    emit DevMngIns->protocolDevRemoved(id, oldMpt);
}

void DeviceWatcher::onProtoDevMounted(const QString &id, const QString &mpt)
{
    d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));

    emit DevMngIns->protocolDevMounted(id, mpt);
}

void DeviceWatcher::onProtoDevUnmounted(const QString &id)
{
    //    auto dev = DeviceHelper::createProtocolDevice(id);
    //    if (dev)
    //        d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));
    //    else
    QString oldMpt = d->allProtocolInfos.value(id).value(DeviceProperty::kMountPoint).toString();
    d->allProtocolInfos.remove(id);

    emit DevMngIns->protocolDevUnmounted(id, oldMpt);
}

QVariantMap DeviceWatcher::getDevInfo(const QString &id, dfmmount::DeviceType type, bool reload)
{
    if (type == DFMMOUNT::DeviceType::kBlockDevice) {
        if (reload) {
            QVariantMap newInfo = DeviceHelper::loadBlockInfo(id);
            if (newInfo.isEmpty())
                return QVariantMap();
            // Xust: fix issue that no device usage display when dfm launched
            // But I still don't understand what's being done here, maybe a more reasonable explanation is needed from xust
            bool isOptical { newInfo[DeviceProperty::kOpticalDrive].toBool() };
            if (!isOptical) {
                const QVariantMap &oldInfo = d->allBlockInfos.value(id, QVariantMap());
                newInfo[DeviceProperty::kSizeFree] = oldInfo.value(DeviceProperty::kSizeFree, 0);
                newInfo[DeviceProperty::kSizeUsed] = oldInfo.value(DeviceProperty::kSizeUsed, 0);
            }
            d->allBlockInfos.insert(id, newInfo);
        }
        return d->allBlockInfos.value(id, QVariantMap());
    } else if (type == DFMMOUNT::DeviceType::kProtocolDevice) {
        if (reload) {
            const auto &&info = DeviceHelper::loadProtocolInfo(id);
            if (!info.value("fake", false).toBool())   // only update with real info.
                d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));
        }

        return d->allProtocolInfos.value(id, QVariantMap());
    }
    return QVariantMap();
}

QStringList DeviceWatcher::getDevIds(dfmmount::DeviceType type)
{
    if (type == DeviceType::kBlockDevice)
        return d->allBlockInfos.keys();
    else if (type == DeviceType::kProtocolDevice)
        return d->allProtocolInfos.keys();
    return {};
}

/*!
 * \brief DeviceWatcher::getSiblings, this function only valid for block devices.
 * \param id
 * \return
 */
QStringList DeviceWatcher::getSiblings(const QString &id)
{
    if (!id.startsWith(kBlockDeviceIdPrefix))
        return {};

    auto mng = DFMMOUNT::DDeviceManager::instance();
    auto blkMonitor = mng->getRegisteredMonitor(DeviceType::kBlockDevice).objectCast<DFMMOUNT::DBlockMonitor>();
    if (!blkMonitor)
        return {};

    const auto &&me = DeviceHelper::loadBlockInfo(id);
    const QString &drive = me.value(DeviceProperty::kDrive).toString();
    auto ret = blkMonitor->resolveDeviceFromDrive(drive);
    ret.sort();
    return ret;
}

DeviceWatcherPrivate::DeviceWatcherPrivate(DeviceWatcher *qq)
    : QObject(qq), q(qq)
{
    connect(DevProxyMng, &DeviceProxyManager::devSizeChanged, this, &DeviceWatcherPrivate::updateStorage, Qt::QueuedConnection);
    DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.mount");
}
