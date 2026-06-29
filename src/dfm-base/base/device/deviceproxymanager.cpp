// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceproxymanager.h"
#include "devicemanager.h"
#include "deviceutils.h"
#include "private/deviceproxymanager_p.h"

#include <QDBusServiceWatcher>
#include <QCoreApplication>

#include <glib.h>

using namespace dfmbase;
static constexpr char kDeviceService[] { "org.deepin.Filemanager.Daemon" };
static constexpr char kDevMngPath[] { "/org/deepin/Filemanager/Daemon/DeviceManager" };

DeviceProxyManager *DeviceProxyManager::instance()
{
    static DeviceProxyManager ins;
    return &ins;
}

const OrgDeepinFilemanagerDaemonDeviceManagerInterface *DeviceProxyManager::getDBusIFace() const
{
    return d->devMngDBus.data();
}

QStringList DeviceProxyManager::getAllBlockIds(GlobalServerDefines::DeviceQueryOptions opts)
{
    if (d->isDBusRuning() && d->devMngDBus) {
        auto &&reply = d->devMngDBus->GetBlockDevicesIdList(opts);
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getAllBlockDevID(opts);
    }
}

QStringList DeviceProxyManager::getAllBlockIdsByUUID(const QStringList &uuids, GlobalServerDefines::DeviceQueryOptions opts)
{
    const auto &&devices = getAllBlockIds(opts);
    QStringList devs;
    for (const auto &id : devices) {
        const auto &&info = queryBlockInfo(id);
        if (uuids.contains(info.value(GlobalServerDefines::DeviceProperty::kUUID).toString()))
            devs << id;
    }
    return devs;
}

QStringList DeviceProxyManager::getAllProtocolIds()
{
    if (d->isDBusRuning() && d->devMngDBus) {
        auto &&reply = d->devMngDBus->GetProtocolDevicesIdList();
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getAllProtocolDevID();
    }
}

QVariantMap DeviceProxyManager::queryBlockInfo(const QString &id, bool reload)
{
    if (d->isDBusRuning() && d->devMngDBus) {
        auto &&reply = d->devMngDBus->QueryBlockDeviceInfo(id, reload);
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getBlockDevInfo(id, reload);
    }
}

QVariantMap DeviceProxyManager::queryProtocolInfo(const QString &id, bool reload)
{
    if (d->isDBusRuning() && d->devMngDBus) {
        auto &&reply = d->devMngDBus->QueryProtocolDeviceInfo(id, reload);
        reply.waitForFinished();
        return reply.value();
    } else {
        return DevMngIns->getProtocolDevInfo(id, reload);
    }
}

void DeviceProxyManager::subscribeUsageMonitoring()
{
    if (d->isDBusRuning() && d->devMngDBus) {
        d->devMngDBus->StartMonitoringUsage();
    } else {
        DevMngIns->startPollingDeviceUsage();
    }
}

void DeviceProxyManager::unsubscribeUsageMonitoring()
{
    if (d->isDBusRuning() && d->devMngDBus) {
        d->devMngDBus->StopMonitoringUsage();
    } else {
        DevMngIns->stopPollingDeviceUsage();
    }
}

void DeviceProxyManager::refreshUsage()
{
    if (d->isDBusRuning() && d->devMngDBus) {
        d->devMngDBus->RefreshDeviceUsage();
    } else {
        DevMngIns->refreshUsage();
    }
}

void DeviceProxyManager::reloadOpticalInfo(const QString &id)
{
    if (d->isDBusRuning() && d->devMngDBus)
        queryBlockInfo(id, true);
    else
        DevMngIns->getBlockDevInfo(id, true);
}

bool DeviceProxyManager::initService()
{
    d->initConnection();
    return isDBusRuning();
}

bool DeviceProxyManager::isDBusRuning()
{
    return d->isDBusRuning();
}

bool DeviceProxyManager::isFileOfExternalMounts(const QString &filePath)
{
    d->initMounts();
    return d->matchMounts(filePath, d->externalMounts,
                          [](const QString &) { return true; });
}

bool DeviceProxyManager::isFileOfProtocolMounts(const QString &filePath)
{
    d->initMounts();
    return d->matchMounts(filePath, d->allMounts,
                          [](const QString &id) { return !id.startsWith(kBlockDeviceIdPrefix); });
}

bool DeviceProxyManager::isFileOfExternalBlockMounts(const QString &filePath)
{
    d->initMounts();
    return d->matchMounts(filePath, d->externalMounts,
                          [](const QString &id) { return id.startsWith(kBlockDeviceIdPrefix); });
}

bool DeviceProxyManager::isFileFromOptical(const QString &filePath)
{
    d->initMounts();
    static const QString kOpticalPrefix = QString(kBlockDeviceIdPrefix) + "sr";
    return d->matchMounts(filePath, d->allMounts,
                          [](const QString &id) { return id.startsWith(kOpticalPrefix); });
}

bool DeviceProxyManager::isMptOfDevice(const QString &filePath, QString &id)
{
    d->initMounts();
    const QString &path = filePath.endsWith("/") ? filePath : filePath + "/";
    QReadLocker lk(&d->lock);
    for (auto it = d->allMounts.constBegin(); it != d->allMounts.constEnd(); ++it) {
        if (it.value().contains(path)) {
            id = it.key();
            return true;
        }
    }
    id.clear();
    return false;
}

QVariantMap DeviceProxyManager::queryDeviceInfoByPath(const QString &path, bool reload)
{
    d->initMounts();
    QString devId, rootDevId, blkMtp;
    QReadLocker lk(&d->lock);
    for (auto it = d->allMounts.constBegin(); it != d->allMounts.constEnd(); ++it) {
        for (const QString &mpt : it.value()) {
            if (mpt == "/") {
                rootDevId = it.key();   // record system disk as fallback
                continue;
            }
            if (path.startsWith(mpt) || (path + "/").startsWith(mpt)) {
                if (blkMtp.isEmpty() || mpt.length() > blkMtp.length()) {
                    devId = it.key();
                    blkMtp = mpt;
                }
                qCDebug(logDFMBase()) << "DeviceProxyManager::queryDeviceInfoByPath path = " << path
                                      << ", devId = " << devId << ", mpt = " << mpt << blkMtp;
            }
        }
    }
    if (devId.isEmpty())
        devId = rootDevId;
    return devId.startsWith(kBlockDeviceIdPrefix)
            ? queryBlockInfo(devId, reload)
            : queryProtocolInfo(devId, reload);
}

DeviceProxyManager::DeviceProxyManager(QObject *parent)
    : QObject(parent), d(new DeviceProxyManagerPrivate(this, parent))
{
}

DeviceProxyManager::~DeviceProxyManager()
{
}

DeviceProxyManagerPrivate::DeviceProxyManagerPrivate(DeviceProxyManager *qq, QObject *parent)
    : QObject(parent), q(qq)
{
    if (qApp) {
        connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
            isShuttingDown.storeRelease(true);
            dbusWatcher.reset();
            devMngDBus.reset();
            disconnCurrentConnections();
        });
    }
}

DeviceProxyManagerPrivate::~DeviceProxyManagerPrivate()
{
}

bool DeviceProxyManagerPrivate::isDBusRuning()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(kDeviceService);
}

void DeviceProxyManagerPrivate::initConnection()
{
    if (isShuttingDown.loadAcquire())
        return;

    dbusWatcher.reset(new QDBusServiceWatcher(kDeviceService, QDBusConnection::sessionBus()));
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceRegistered, q, [this] {
        if (isShuttingDown.loadAcquire())
            return;
        connectToDBus();
        emit q->devMngDBusRegistered();
        qCInfo(logDFMBase) << "Device manager DBus service registered, switching to DBus connection";
    });
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceUnregistered, q, [this] {
        if (isShuttingDown.loadAcquire()) {
            devMngDBus.reset();
            return;
        }
        devMngDBus.reset();
        connectToAPI();
        emit q->devMngDBusUnregistered();
        qCInfo(logDFMBase) << "Device manager DBus service unregistered, switching to direct API connection";
    });

    if (isDBusRuning()) {
        qCInfo(logDFMBase) << "Device manager DBus service is available, connecting to DBus";
        connectToDBus();
    } else {
        qCInfo(logDFMBase) << "Device manager DBus service not available, connecting to API directly";
        connectToAPI();
    }
}

void DeviceProxyManagerPrivate::initMounts()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        using namespace GlobalServerDefines;

        auto func = [this](const QStringList &devs,
                           std::function<QVariantMap(DeviceProxyManager *, const QString &, bool)> query,
                           bool pass = false) {
            for (const auto &dev : devs) {
                auto &&info = query(q, dev, false);
                // Prefer the plural `kMountPoints` which carries the full list of mount
                // points of a device (e.g. multi-mount LVM devices). Fall back to the
                // singular `kMountPoint` for daemons that only return a single value.
                auto mpts = info.value(DeviceProperty::kMountPoints).toStringList();
                if (mpts.isEmpty()) {
                    auto single = info.value(DeviceProperty::kMountPoint).toString();
                    if (!single.isEmpty())
                        mpts << single;
                }

                qCDebug(logDFMBase()) << "DeviceProxyManager initMounts dev =" << dev
                                      << "mountPoints =" << mpts;

                const bool isBlock = info.value(DeviceProperty::kId).toString().startsWith(kBlockDeviceIdPrefix);
                const bool isExternal = pass || isExternalBlock(info);
                for (const auto &raw : mpts) {
                    if (raw.isEmpty())
                        continue;
                    // Skip dlnfs mount points for non-block devices (protocol devices).
                    if (DeviceUtils::isMountPointOfDlnfs(raw) && !isBlock)
                        continue;
                    const QString &mpt = canonicalMountPoint(raw);

                    QWriteLocker lk(&lock);
                    if (!allMounts[dev].contains(mpt))
                        allMounts[dev].append(mpt);
                    if (isExternal && !externalMounts[dev].contains(mpt))
                        externalMounts[dev].append(mpt);
                }
            }
        };

        auto blks = q->getAllBlockIds();
        auto protos = q->getAllProtocolIds();
        func(blks, &DeviceProxyManager::queryBlockInfo);
        // All protocol devices should be added to externalMounts
        func(protos, &DeviceProxyManager::queryProtocolInfo, true);
    });
}

QString DeviceProxyManagerPrivate::canonicalMountPoint(const QString &mpt) const
{
    if (mpt.isEmpty())
        return {};

    QString mountPoint = mpt;
    g_autofree gchar *canonical_mpt = g_canonicalize_filename(mountPoint.toStdString().c_str(), NULL);
    if (canonical_mpt)
        mountPoint = canonical_mpt;

    mountPoint = mountPoint.endsWith("/") ? mountPoint : mountPoint + "/";
    return mountPoint;
}

/*!
 * \brief DeviceProxyManagerPrivate::isExternalBlock Check if device is an external removable block device
 * For encrypted devices (LUKS/dm-crypt), checks the underlying physical device's removable property
 * \param info Device info from UDisks2
 * \return true if external removable device
 */
bool DeviceProxyManagerPrivate::isExternalBlock(const QVariantMap &info) const
{
    using namespace GlobalServerDefines;

    if (info.isEmpty()) {
        qCWarning(logDFMBase) << "Empty device info provided to isExternalBlock";
        return false;
    }

    const QString cryptoBacking = info.value(DeviceProperty::kCryptoBackingDevice).toString();
    if (!cryptoBacking.isEmpty() && cryptoBacking != "/") {
        // Encrypted device: check underlying physical device
        auto backingInfo = q->queryBlockInfo(cryptoBacking);
        if (backingInfo.isEmpty()) {
            qCWarning(logDFMBase) << "Failed to get backing device info for: " << cryptoBacking;
            return false;
        }
        return backingInfo.value(DeviceProperty::kRemovable).toBool()
                && !DeviceUtils::isBuiltInDisk(backingInfo);
    }

    // Normal device
    return info.value(DeviceProperty::kRemovable).toBool()
            && !DeviceUtils::isBuiltInDisk(info);
}

void DeviceProxyManagerPrivate::connectToDBus()
{
    if (isShuttingDown.loadAcquire())
        return;

    if (currentConnectionType == kDBusConnecting)
        return;

    disconnCurrentConnections();

    devMngDBus.reset(new DeviceManagerInterface(kDeviceService, kDevMngPath, QDBusConnection::sessionBus(), this));
    auto ptr = devMngDBus.data();
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDriveAdded, q, &DeviceProxyManager::blockDriveAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDriveRemoved, q, &DeviceProxyManager::blockDriveRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceAdded, q, &DeviceProxyManager::blockDevAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceRemoved, q, &DeviceProxyManager::blockDevRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceMounted, q, &DeviceProxyManager::blockDevMounted);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceUnmounted, q, &DeviceProxyManager::blockDevUnmounted);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceLocked, q, &DeviceProxyManager::blockDevLocked);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceUnlocked, q, &DeviceProxyManager::blockDevUnlocked);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceFilesystemAdded, q, &DeviceProxyManager::blockDevFsAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceFilesystemRemoved, q, &DeviceProxyManager::blockDevFsRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDevicePropertyChanged, q, [this](const QString &id, const QString &property, const QDBusVariant &value) {
        emit this->q->blockDevPropertyChanged(id, property, value.variant());
    });

    connections << q->connect(ptr, &DeviceManagerInterface::SizeUsedChanged, q, &DeviceProxyManager::devSizeChanged);

    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceAdded, q, &DeviceProxyManager::protocolDevAdded);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceRemoved, q, &DeviceProxyManager::protocolDevRemoved);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceMounted, q, &DeviceProxyManager::protocolDevMounted);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceUnmounted, q, &DeviceProxyManager::protocolDevUnmounted);

    // *Removed: whole device gone — discard oldMpt so removeMounts clears all mount points.
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceRemoved, this, [this](const QString &id, const QString &) { removeMounts(id); });
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceMounted, this, &DeviceProxyManagerPrivate::addMounts);
    // *Unmounted: one mount point gone — pass oldMpt so removeMounts targets that specific entry.
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceUnmounted, this, &DeviceProxyManagerPrivate::removeMounts);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceRemoved, this, [this](const QString &id, const QString &) { removeMounts(id); });
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceMounted, this, &DeviceProxyManagerPrivate::addMounts);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceUnmounted, this, &DeviceProxyManagerPrivate::removeMounts);

    // redundant signal. this signal is emitted before the mount callback invoked, to make sure the cache is updated before use.
    connections << q->connect(DevMngIns, &DeviceManager::blockDevMountedManually, this, &DeviceProxyManagerPrivate::addMounts);

    currentConnectionType = kDBusConnecting;
    qCInfo(logDFMBase) << "Device proxy manager connected to DBus service successfully";
}

void DeviceProxyManagerPrivate::connectToAPI()
{
    if (isShuttingDown.loadAcquire())
        return;

    if (currentConnectionType == kAPIConnecting)
        return;
    disconnCurrentConnections();

    devMngDBus.reset();
    auto ptr = DevMngIns;
    connections << q->connect(ptr, &DeviceManager::blockDriveAdded, q, &DeviceProxyManager::blockDriveAdded);
    connections << q->connect(ptr, &DeviceManager::blockDriveRemoved, q, &DeviceProxyManager::blockDriveRemoved);
    connections << q->connect(ptr, &DeviceManager::blockDevAdded, q, &DeviceProxyManager::blockDevAdded);
    connections << q->connect(ptr, &DeviceManager::blockDevRemoved, q, &DeviceProxyManager::blockDevRemoved);
    connections << q->connect(ptr, &DeviceManager::blockDevMounted, q, &DeviceProxyManager::blockDevMounted);
    connections << q->connect(ptr, &DeviceManager::blockDevUnmounted, q, &DeviceProxyManager::blockDevUnmounted);
    connections << q->connect(ptr, &DeviceManager::blockDevLocked, q, &DeviceProxyManager::blockDevLocked);
    connections << q->connect(ptr, &DeviceManager::blockDevUnlocked, q, &DeviceProxyManager::blockDevUnlocked);
    connections << q->connect(ptr, &DeviceManager::blockDevFsAdded, q, &DeviceProxyManager::blockDevFsAdded);
    connections << q->connect(ptr, &DeviceManager::blockDevFsRemoved, q, &DeviceProxyManager::blockDevFsRemoved);
    connections << q->connect(ptr, &DeviceManager::blockDevPropertyChanged, q, &DeviceProxyManager::blockDevPropertyChanged);

    connections << q->connect(ptr, &DeviceManager::devSizeChanged, q, &DeviceProxyManager::devSizeChanged);

    connections << q->connect(ptr, &DeviceManager::protocolDevAdded, q, &DeviceProxyManager::protocolDevAdded);
    connections << q->connect(ptr, &DeviceManager::protocolDevRemoved, q, &DeviceProxyManager::protocolDevRemoved);
    connections << q->connect(ptr, &DeviceManager::protocolDevMounted, q, &DeviceProxyManager::protocolDevMounted);
    connections << q->connect(ptr, &DeviceManager::protocolDevUnmounted, q, &DeviceProxyManager::protocolDevUnmounted);

    // *Removed: whole device gone — discard oldMpt so removeMounts clears all mount points.
    connections << q->connect(ptr, &DeviceManager::blockDevRemoved, this, [this](const QString &id, const QString &) { removeMounts(id); });
    connections << q->connect(ptr, &DeviceManager::blockDevMounted, this, &DeviceProxyManagerPrivate::addMounts);
    // *Unmounted: one mount point gone — pass oldMpt so removeMounts targets that specific entry.
    connections << q->connect(ptr, &DeviceManager::blockDevUnmounted, this, &DeviceProxyManagerPrivate::removeMounts);
    connections << q->connect(ptr, &DeviceManager::protocolDevRemoved, this, [this](const QString &id, const QString &) { removeMounts(id); });
    connections << q->connect(ptr, &DeviceManager::protocolDevMounted, this, &DeviceProxyManagerPrivate::addMounts);
    connections << q->connect(ptr, &DeviceManager::protocolDevUnmounted, this, &DeviceProxyManagerPrivate::removeMounts);

    // redundant signal. this signal is emitted before the mount callback invoked, to make sure the cache is updated before use.
    connections << q->connect(ptr, &DeviceManager::blockDevMountedManually, this, &DeviceProxyManagerPrivate::addMounts);

    currentConnectionType = kAPIConnecting;
    qCInfo(logDFMBase) << "Device proxy manager connected to API directly";

    DevMngIns->startMonitor();
}

void DeviceProxyManagerPrivate::disconnCurrentConnections()
{
    for (const auto &connection : connections)
        q->disconnect(connection);
    connections.clear();
    currentConnectionType = kNoneConnection;
}

bool DeviceProxyManagerPrivate::matchMounts(const QString &filePath,
                                            const QMap<QString, QStringList> &mounts,
                                            const std::function<bool(const QString &)> &devFilter)
{
    if (filePath.isEmpty())
        return false;
    const QString &path = filePath.endsWith("/") ? filePath : filePath + "/";
    QReadLocker lk(&lock);
    for (auto it = mounts.constBegin(); it != mounts.constEnd(); ++it) {
        if (!devFilter(it.key()))
            continue;
        for (const QString &mpt : it.value()) {
            if (path.startsWith(mpt))
                return true;
        }
    }
    return false;
}

void DeviceProxyManagerPrivate::addMounts(const QString &id, const QString &mpt)
{
    QString p = canonicalMountPoint(mpt);
    if (!id.startsWith(kBlockDeviceIdPrefix) && DeviceUtils::isMountPointOfDlnfs(p))
        return;

    // NOTE: Moving positions may cause deadlock
    Q_EMIT q->mountPointAboutToAdded(mpt);

    {
        QWriteLocker lk(&lock);
        if (!allMounts[id].contains(p))
            allMounts[id].append(p);
        bool isExternal = false;
        if (id.startsWith(kBlockDeviceIdPrefix)) {
            auto &&info = q->queryBlockInfo(id);
            isExternal = isExternalBlock(info);
        } else {
            isExternal = true;
        }
        if (isExternal && !externalMounts[id].contains(p))
            externalMounts[id].append(p);
    }

    Q_EMIT q->mountPointAdded(mpt);
}

void DeviceProxyManagerPrivate::removeMounts(const QString &id, const QString &mpt)
{
    // NOTE: signals are emitted outside the write lock to avoid deadlock.
    if (!mpt.isEmpty()) {
        // A specific mount point was unmounted: remove only that one.
        const QString p = canonicalMountPoint(mpt);
        // Emit using the raw mpt as callers expect the original value.
        Q_EMIT q->mountPointAboutToRemoved(mpt);

        bool devGone = false;
        {
            QWriteLocker lk(&lock);
            auto allIt = allMounts.find(id);
            if (allIt != allMounts.end()) {
                allIt.value().removeAll(p);
                if (allIt.value().isEmpty()) {
                    allMounts.erase(allIt);
                    devGone = true;
                }
            }
            auto extIt = externalMounts.find(id);
            if (extIt != externalMounts.end()) {
                extIt.value().removeAll(p);
                if (extIt.value().isEmpty() || devGone)
                    externalMounts.erase(extIt);
            }
        }
        Q_EMIT q->mountPointRemoved(mpt);
    } else {
        // The whole device was removed: emit AboutToRemoved per mount point, drop the
        // key, then emit Removed per mount point (cache is gone by then).
        QStringList mpts;
        {
            QReadLocker lk(&lock);
            mpts = allMounts.value(id);
        }
        for (const QString &one : mpts)
            Q_EMIT q->mountPointAboutToRemoved(one);
        {
            QWriteLocker lk(&lock);
            allMounts.remove(id);
            externalMounts.remove(id);
        }
        for (const QString &one : mpts)
            Q_EMIT q->mountPointRemoved(one);
    }
}
