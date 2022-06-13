/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "devicewatcher.h"
#include "devicewatcher_p.h"
#include "devicehelper.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <QVariantMap>
#include <QDebug>
#include <QStorageInfo>
#include <QtConcurrent>

#include <dfm-mount/dmount.h>
#include <dfm-burn/dopticaldiscinfo.h>
#include <dfm-burn/dopticaldiscmanager.h>

DFMBASE_USE_NAMESPACE
DFM_MOUNT_USE_NS
using namespace GlobalServerDefines;

DeviceWatcher::DeviceWatcher(QObject *parent)
    : QObject(parent), d(new DeviceWatcherPrivate(this))
{
}

DeviceWatcher::~DeviceWatcher()
{
}

void DeviceWatcher::startPollingUsage()
{
    if (d->pollingTimer.isActive())
        return;
    d->queryUsage();
    connect(&d->pollingTimer, &QTimer::timeout, this, [this] { d->queryUsage(); });
    d->pollingTimer.start(d->kPollingInterval);
}

void DeviceWatcher::stopPollingUsage()
{
    d->pollingTimer.stop();
    disconnect(&d->pollingTimer);
}

void DeviceWatcherPrivate::queryUsage()
{
    QtConcurrent::run([this] {
        queryUsage(DeviceType::kBlockDevice, allBlockInfos);
        queryUsage(DeviceType::kProtocolDevice, allProtocolInfos);
    });
}

void DeviceWatcherPrivate::queryUsage(DeviceType type, const QHash<QString, QVariantMap> &datas)
{
    for (const auto &item : datas) {
        auto id = item.value(DeviceProperty::kId).toString();
        auto mpt = item.value(DeviceProperty::kMountPoint).toString();
        queryUsage(id, mpt, type, true);
    }
}

void DeviceWatcherPrivate::queryUsage(const QString &id, const QString &mpt, DeviceType type, bool notifyIfChanged)
{
    if (mpt.isEmpty())
        return;

    using namespace GlobalServerDefines;
    if (type == DeviceType::kBlockDevice) {
        if (!allBlockInfos.contains(id))
            return;

        const auto &info = allBlockInfos.value(id);
        QVariantMap data { { DeviceProperty::kDevice, info.value(DeviceProperty::kDevice) } };
        quint64 bytesAvai;
        if (info.value(DeviceProperty::kOpticalDrive).toBool()) {
            DeviceHelper::readOpticalInfo(data);
            bytesAvai = data[DeviceProperty::kSizeTotal].toULongLong() - data[DeviceProperty::kSizeUsed].toULongLong();
        } else {
            QStorageInfo si(mpt);
            bytesAvai = si.bytesAvailable();
        }

        if (bytesAvai != info.value(DeviceProperty::kSizeFree)) {
            auto &rinfo = allBlockInfos[id];
            {
                QMutexLocker lk(&blkMtx);
                rinfo[DeviceProperty::kSizeFree] = bytesAvai;
                rinfo[DeviceProperty::kSizeUsed] = rinfo[DeviceProperty::kSizeTotal].toULongLong() - bytesAvai;
            }
            if (notifyIfChanged)
                emit DevMngIns->devSizeChanged(id, info.value(DeviceProperty::kSizeTotal).toULongLong(), bytesAvai);
        }
    } else if (type == DeviceType::kProtocolDevice) {
        if (!allProtocolInfos.contains(id))
            return;

        const auto &info = allProtocolInfos.value(id);
        auto dev = DeviceHelper::createProtocolDevice(id);
        quint64 avai = dev->sizeFree();
        quint64 used = dev->sizeUsage();
        quint64 total = dev->sizeTotal();
        if (avai != info.value(DeviceProperty::kSizeFree)
            || used != info.value(DeviceProperty::kSizeUsed)
            || total != info.value(DeviceProperty::kSizeTotal)) {
            auto &rinfo = allProtocolInfos[id];
            {
                QMutexLocker lk(&protoMtx);
                rinfo[DeviceProperty::kSizeFree] = avai;
                rinfo[DeviceProperty::kSizeUsed] = used;
                rinfo[DeviceProperty::kSizeTotal] = total;
            }
            if (notifyIfChanged)
                emit DevMngIns->devSizeChanged(id, info.value(DeviceProperty::kSizeTotal).toULongLong(), avai);
        }
    }
}

void DeviceWatcher::initDevDatas()
{
    auto mng = DDeviceManager::instance();
    auto &&devs = mng->devices();
    for (const auto &dev : devs.value(DeviceType::kBlockDevice))
        d->allBlockInfos.insert(dev, DeviceHelper::loadBlockInfo(dev));
    for (const auto &dev : devs.value(DeviceType::kProtocolDevice))
        d->allProtocolInfos.insert(dev, DeviceHelper::loadProtocolInfo(dev));
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
    QVariantMap data = DeviceHelper::loadBlockInfo(id);
    if (data.value(DeviceProperty::kId).toString().isEmpty())
        return;
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
    if (sizeChanged) {
        DeviceHelper::persistentOpticalInfo(data);
        emit DevMngIns->devSizeChanged(id, data[DeviceProperty::kSizeTotal].toULongLong(), data[DeviceProperty::kSizeFree].toULongLong());
        DevProxyMng->reloadOpticalInfo(id);
    }
}

void DeviceWatcher::startWatch()
{
    if (d->isWatching) {
        qInfo() << "watching device changes now...";
        return;
    }

    auto mng = DDeviceManager::instance();
    mng->startMonitorWatch();
    auto blkMonitor = mng->getRegisteredMonitor(DeviceType::kBlockDevice).objectCast<DBlockMonitor>();
    if (!blkMonitor) {
        qWarning() << "block monitor is not valid!!!";
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
    }

    auto protoMonitor = mng->getRegisteredMonitor(DeviceType::kProtocolDevice).objectCast<DProtocolMonitor>();
    if (!protoMonitor) {
        qWarning() << "protocol monitor is not valid!!!";
    } else {
        auto ptr = protoMonitor.data();
        d->connections << connect(ptr, &DProtocolMonitor::deviceAdded, this, &DeviceWatcher::onProtoDevAdded);
        d->connections << connect(ptr, &DProtocolMonitor::deviceRemoved, this, &DeviceWatcher::onProtoDevRemoved);
        d->connections << connect(ptr, &DProtocolMonitor::mountAdded, this, &DeviceWatcher::onProtoDevMounted);
        d->connections << connect(ptr, &DProtocolMonitor::mountRemoved, this, &DeviceWatcher::onProtoDevUnmounted);

        d->isWatching = true;
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
    qDebug() << "new block device added: " << id;
    auto dev = DeviceHelper::createBlockDevice(id);
    {
        QMutexLocker lk(&d->blkMtx);
        d->allBlockInfos.insert(id, DeviceHelper::loadBlockInfo(dev));
    }

    emit DevMngIns->blockDevAdded(id);
    DevMngIns->doAutoMount(id, DeviceType::kBlockDevice);
}

void DeviceWatcher::onBlkDevRemoved(const QString &id)
{
    qDebug() << "block device removed: " << id;
    {
        QMutexLocker lk(&d->blkMtx);
        d->allBlockInfos.remove(id);
    }
    emit DevMngIns->blockDevRemoved(id);
}

void DeviceWatcher::onBlkDevMounted(const QString &id, const QString &mpt)
{
    d->queryUsage(id, mpt, DeviceType::kBlockDevice, true);
    emit DevMngIns->blockDevMounted(id, mpt);
}

void DeviceWatcher::onBlkDevUnmounted(const QString &id)
{
    emit DevMngIns->blockDevUnmounted(id);
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
    {
        // when a filesystem interface is added, lots of property may changed, so just reload the data
        // this will not happen frequently
        QMutexLocker lk(&d->blkMtx);
        d->allBlockInfos.insert(id, DeviceHelper::loadBlockInfo(id));
    }
    emit DevMngIns->blockDevFsAdded(id);
    using namespace GlobalServerDefines;
    emit DevMngIns->blockDevPropertyChanged(id, DeviceProperty::kHasFileSystem, true);
}

void DeviceWatcher::onBlkDevFsRemoved(const QString &id)
{
    auto data = DeviceHelper::loadBlockInfo(id);
    if (!data.isEmpty()) {
        // when a filesystem interface is added, lots of property may changed, so just reload the data
        // this will not happen frequently
        QMutexLocker lk(&d->blkMtx);
        d->allBlockInfos.insert(id, data);
    }
    emit DevMngIns->blockDevFsRemoved(id);
    using namespace GlobalServerDefines;
    emit DevMngIns->blockDevPropertyChanged(id, DeviceProperty::kHasFileSystem, false);
}

void DeviceWatcher::onBlkDevPropertiesChanged(const QString &id, const QMap<Property, QVariant> &changes)
{
    for (auto iter = changes.cbegin(); iter != changes.cend(); ++iter) {
        const auto &property = iter.key();
        const auto &var = iter.value();
        auto name = DeviceHelper::castFromDFMMountProperty(property);
        if (name.isEmpty()) {
            qInfo() << Utils::getNameByProperty(property) << "has no mapped device name";
            qInfo() << "changed value is: " << var;
            continue;
        } else {
            QVariantMap &item = d->allBlockInfos[id];
            {
                QMutexLocker lk(&d->blkMtx);
                item[name] = var;
                if (name == DeviceProperty::kMountPoints)
                    item[DeviceProperty::kMountPoint] = var.toStringList().isEmpty() ? "" : var.toStringList().first();
                if (name == DeviceProperty::kOptical && !var.toBool()) {
                    item[DeviceProperty::kOpticalMediaType] = "";
                    item[DeviceProperty::kOpticalWriteSpeed] = QStringList();
                    item[DeviceProperty::kSizeTotal] = 0;
                    item[DeviceProperty::kSizeFree] = 0;
                    item[DeviceProperty::kSizeUsed] = 0;
                }
            }
            emit DevMngIns->blockDevPropertyChanged(id, name, var);
        }
    }
}

void DeviceWatcher::onProtoDevAdded(const QString &id)
{
    qDebug() << "new protocol device added: " << id;
    auto dev = DeviceHelper::createProtocolDevice(id);
    {
        QMutexLocker lk(&d->protoMtx);
        d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));
    }

    emit DevMngIns->protocolDevAdded(id);
    DevMngIns->doAutoMount(id, DeviceType::kProtocolDevice);
}

void DeviceWatcher::onProtoDevRemoved(const QString &id)
{
    qDebug() << "protocol device removed: " << id;
    {
        QMutexLocker lk(&d->protoMtx);
        d->allProtocolInfos.remove(id);
    }
    emit DevMngIns->protocolDevRemoved(id);
}

void DeviceWatcher::onProtoDevMounted(const QString &id, const QString &mpt)
{
    auto dev = DeviceHelper::createProtocolDevice(id);
    {
        QMutexLocker lk(&d->protoMtx);
        d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));
    }

    emit DevMngIns->protocolDevMounted(id, mpt);
}

void DeviceWatcher::onProtoDevUnmounted(const QString &id)
{
    QMutexLocker lk(&d->protoMtx);
    //    auto dev = DeviceHelper::createProtocolDevice(id);
    //    if (dev)
    //        d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));
    //    else
    d->allProtocolInfos.remove(id);

    emit DevMngIns->protocolDevUnmounted(id);
}

QVariantMap DeviceWatcher::getDevInfo(const QString &id, dfmmount::DeviceType type, bool reload)
{
    if (type == DFMMOUNT::DeviceType::kBlockDevice) {
        if (reload) {
            QMutexLocker lk(&d->blkMtx);
            d->allBlockInfos.insert(id, DeviceHelper::loadBlockInfo(id));
        }
        return d->allBlockInfos.value(id);
    } else if (type == DFMMOUNT::DeviceType::kProtocolDevice) {
        if (reload) {
            QMutexLocker lk(&d->protoMtx);
            d->allProtocolInfos.insert(id, DeviceHelper::loadProtocolInfo(id));
        }
        return d->allProtocolInfos.value(id);
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
    : q(qq)
{
}
