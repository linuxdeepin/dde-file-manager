// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceproxymanager.h"
#include "devicemanager.h"
#include "deviceutils.h"
#include "private/deviceproxymanager_p.h"

#include <QDBusServiceWatcher>

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
    QTimer::singleShot(1000, this, [this] { d->initMounts(); });
    return isDBusRuning();
}

bool DeviceProxyManager::isDBusRuning()
{
    return d->isDBusRuning();
}

bool DeviceProxyManager::isFileOfExternalMounts(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    d->initMounts();
    const QStringList &&mpts = d->externalMounts.values();
    QString path = filePath.endsWith("/") ? filePath : filePath + "/";
    auto ret = std::find_if(mpts.cbegin(), mpts.cend(), [path](const QString &mpt) { return path.startsWith(mpt); });
    return ret != mpts.cend();
}

bool DeviceProxyManager::isFileOfProtocolMounts(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    d->initMounts();
    const QString &path = filePath.endsWith("/") ? filePath : filePath + "/";
    QReadLocker lk(&d->lock);
    for (auto iter = d->allMounts.constKeyValueBegin(); iter != d->allMounts.constKeyValueEnd(); ++iter) {
        if (!iter.base().key().startsWith(kBlockDeviceIdPrefix) && path.startsWith(iter.base().value()))
            return true;
    }
    return false;
}

bool DeviceProxyManager::isFileOfExternalBlockMounts(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    d->initMounts();
    const QString &path = filePath.endsWith("/") ? filePath : filePath + "/";
    QReadLocker lk(&d->lock);
    for (auto iter = d->externalMounts.constKeyValueBegin(); iter != d->externalMounts.constKeyValueEnd(); ++iter) {
        if (iter.base().key().startsWith(kBlockDeviceIdPrefix) && path.startsWith(iter.base().value()))
            return true;
    }
    return false;
}

bool DeviceProxyManager::isFileFromOptical(const QString &filePath)
{
    d->initMounts();
    const QString &path = filePath.endsWith("/") ? filePath : filePath + "/";
    QReadLocker lk(&d->lock);
    for (auto iter = d->allMounts.constKeyValueBegin(); iter != d->allMounts.constKeyValueEnd(); ++iter) {
        if (iter.base().key().startsWith(QString(kBlockDeviceIdPrefix) + "sr") && path.startsWith(iter.base().value()))
            return true;
    }
    return false;
}

bool DeviceProxyManager::isMptOfDevice(const QString &filePath, QString &id)
{
    d->initMounts();
    const QString &path = filePath.endsWith("/") ? filePath : filePath + "/";
    QReadLocker lk(&d->lock);
    id = d->allMounts.key(path, "");
    return !id.isEmpty();
}

QVariantMap DeviceProxyManager::queryDeviceInfoByPath(const QString &path, bool reload)
{
    d->initMounts();
    QString devId, rootDevId;
    QReadLocker lk(&d->lock);
    for (auto it = d->allMounts.begin(); it != d->allMounts.end(); it++) {
        if (it.value() == "/") {
            rootDevId = it.key();
            continue;
        }
        if (path.startsWith(it.value())
            || (path + "/").startsWith(it.value())) {
            devId = it.key();
            break;
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
    dbusWatcher.reset(new QDBusServiceWatcher(kDeviceService, QDBusConnection::sessionBus()));
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceRegistered, q, [this] {
        connectToDBus();
        emit q->devMngDBusRegistered();
        qCInfo(logDFMBase) << "Device manager DBus service registered, switching to DBus connection";
    });
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceUnregistered, q, [this] {
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
                auto mpt = info.value(DeviceProperty::kMountPoint).toString();
                if (!mpt.isEmpty()) {
                    if (DeviceUtils::isMountPointOfDlnfs(mpt) && !info.value(DeviceProperty::kId).toString().startsWith(kBlockDeviceIdPrefix))
                        continue;
                    mpt = canonicalMountPoint(mpt);

                    QWriteLocker lk(&lock);
                    if (pass || isExternalBlock(info))
                        externalMounts.insert(dev, mpt);
                    allMounts.insert(dev, mpt);
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

    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceRemoved, this, &DeviceProxyManagerPrivate::removeMounts);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceMounted, this, &DeviceProxyManagerPrivate::addMounts);
    connections << q->connect(ptr, &DeviceManagerInterface::BlockDeviceUnmounted, this, &DeviceProxyManagerPrivate::removeMounts);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceRemoved, this, &DeviceProxyManagerPrivate::removeMounts);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceMounted, this, &DeviceProxyManagerPrivate::addMounts);
    connections << q->connect(ptr, &DeviceManagerInterface::ProtocolDeviceUnmounted, this, &DeviceProxyManagerPrivate::removeMounts);

    // redundant signal. this signal is emitted before the mount callback invoked, to make sure the cache is updated before use.
    connections << q->connect(DevMngIns, &DeviceManager::blockDevMountedManually, this, &DeviceProxyManagerPrivate::addMounts);

    currentConnectionType = kDBusConnecting;
    qCInfo(logDFMBase) << "Device proxy manager connected to DBus service successfully";
}

void DeviceProxyManagerPrivate::connectToAPI()
{
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

    connections << q->connect(ptr, &DeviceManager::blockDevRemoved, this, &DeviceProxyManagerPrivate::removeMounts);
    connections << q->connect(ptr, &DeviceManager::blockDevMounted, this, &DeviceProxyManagerPrivate::addMounts);
    connections << q->connect(ptr, &DeviceManager::blockDevUnmounted, this, &DeviceProxyManagerPrivate::removeMounts);
    connections << q->connect(ptr, &DeviceManager::protocolDevRemoved, this, &DeviceProxyManagerPrivate::removeMounts);
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

void DeviceProxyManagerPrivate::addMounts(const QString &id, const QString &mpt)
{
    QString p = canonicalMountPoint(mpt);
    if (!id.startsWith(kBlockDeviceIdPrefix) && DeviceUtils::isMountPointOfDlnfs(p))
        return;

    // NOTE: Moving positions may cause deadlock
    Q_EMIT q->mountPointAboutToAdded(mpt);

    {
        QWriteLocker lk(&lock);
        if (id.startsWith(kBlockDeviceIdPrefix)) {
            auto &&info = q->queryBlockInfo(id);
            if (isExternalBlock(info))
                externalMounts.insert(id, p);
        } else {
            externalMounts.insert(id, p);
        }
        allMounts.insert(id, p);
    }

    Q_EMIT q->mountPointAdded(mpt);
}

void DeviceProxyManagerPrivate::removeMounts(const QString &id)
{
    // NOTE: Moving positions may cause deadlock
    QString mpt;
    {
        QReadLocker locker(&lock);
        mpt = externalMounts.value(id);
    }
    Q_EMIT q->mountPointAboutToRemoved(mpt);

    {
        QWriteLocker lk(&lock);
        externalMounts.remove(id);
        allMounts.remove(id);
    }
    Q_EMIT q->mountPointRemoved(mpt);
}
