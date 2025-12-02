// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicemanager.h"
#include "deviceutils.h"
#include "private/devicemanager_p.h"
#include "private/devicehelper.h"
#include "private/devicewatcher.h"
#include "private/discdevicescanner.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h>

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QApplication>

#include <dfm-mount/dmount.h>
#include <mutex>
#include <unistd.h>

using namespace dfmbase;
DFM_MOUNT_USE_NS
using namespace GlobalServerDefines;
using namespace GlobalDConfDefines::ConfigPath;

static constexpr char kSavePasswd[] { "savePasswd" };
static constexpr char kStashedSmbDevices[] { "StashedSmbDevices" };
static constexpr char kSavedPasswordType[] { "SavedPasswordType" };

static constexpr char kDaemonService[] { "org.deepin.Filemanager.MountControl" };
static constexpr char kDaemonPath[] { "/org/deepin/Filemanager/MountControl" };
static constexpr char kDaemonMountPath[] { "/org/deepin/Filemanager/MountControl" };
static constexpr char kDaemonMountIface[] { "org.deepin.Filemanager.MountControl" };
static constexpr char kDaemonIntroIface[] { "org.freedesktop.DBus.Introspectable" };
static constexpr char kDaemonIntroMethod[] { "Introspect" };

// DLNFS service constants
static constexpr char kDlnfsService[] { "org.deepin.dlnfs.Control" };
static constexpr char kDlnfsPath[] { "/org/deepin/dlnfs/Control" };
static constexpr char kDlnfsIface[] { "org.deepin.dlnfs.Control" };

DeviceManager *DeviceManager::instance()
{
    static DeviceManager ins;
    return &ins;
}

// DeviceManager instance might be used in different process,
// but auto mount should not be enabled in different process
// enable auto mount explicitly
void DeviceManager::enableBlockAutoMount()
{
    d->autoMountBlock = true;
    qCInfo(logDFMBase) << "Block device auto mount enabled";
}

QStringList DeviceManager::getAllBlockDevID(DeviceQueryOptions opts)
{
    auto ret { d->watcher->getDevIds(DeviceType::kBlockDevice) };
    ret.sort();
    if (opts.testFlag(DeviceQueryOption::kNoCondition))
        return ret;

    QString errMsg;
    QStringList filteredRet;
    for (const auto &id : ret) {
        const auto &&data = d->watcher->getDevInfo(id, DeviceType::kBlockDevice, false);
        if (opts.testFlag(DeviceQueryOption::kMounted)
            && data.value(DeviceProperty::kMountPoint).toString().isEmpty())
            continue;
        if (opts.testFlag(DeviceQueryOption::kRemovable)
            && !data.value(DeviceProperty::kRemovable).toBool())
            continue;
        if (opts.testFlag(DeviceQueryOption::kMountable)
            && !DeviceHelper::isMountableBlockDev(data, errMsg))
            continue;
        if (opts.testFlag(DeviceQueryOption::kNotIgnored)
            && data.value(DeviceProperty::kHintIgnore).toBool())
            continue;
        if (opts.testFlag(DeviceQueryOption::kNotMounted)
            && !data.value(DeviceProperty::kMountPoint).toString().isEmpty())
            continue;
        if (opts.testFlag(DeviceQueryOption::kOptical)
            && !data.value(DeviceProperty::kOptical).toBool())
            continue;
        if (opts.testFlag(DeviceQueryOption::kSystem)
            && !DeviceUtils::isBuiltInDisk(data))
            continue;
        if (opts.testFlag(DeviceQueryOption::kLoop)
            && !data.value(DeviceProperty::kIsLoopDevice).toBool())
            continue;
        filteredRet << id;
    }
    return filteredRet;
}

QStringList DeviceManager::getAllProtocolDevID()
{
    auto ret { d->watcher->getDevIds(DeviceType::kProtocolDevice) };
    ret.sort();
    return ret;
}

QVariantMap DeviceManager::getBlockDevInfo(const QString &id, bool needReload)
{
    return d->watcher->getDevInfo(id, DeviceType::kBlockDevice, needReload);
}

QVariantMap DeviceManager::getProtocolDevInfo(const QString &id, bool needReload)
{
    return d->watcher->getDevInfo(id, DeviceType::kProtocolDevice, needReload);
}

void DeviceManager::mountBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType1 cb, int timeout)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "Failed to create block device for mounting:" << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError), "");
        Q_EMIT blockDevMountResult(id, false);
        return;
    }

    QVariantMap options = opts;
    const QStringList winFS { "ntfs", "vfat", "exfat" };
    if (winFS.contains(dev->fileSystem())) {
        auto optStr = options.value("options").toString();
        optStr.prepend("dmask=000,fmask=000,");
        if (optStr.endsWith(",")) optStr.chop(1);
        options.insert("options", optStr);
        qCDebug(logDFMBase) << "Windows filesystem detected, mount options adjusted for device:" << id << "filesystem:" << dev->fileSystem();
    }

    if (dev->optical()) {
        if (d->isMountingOptical) {
            qCWarning(logDFMBase) << "Another optical device is currently being mounted, request rejected for device:" << id;
            Q_EMIT blockDevMountResult(id, false);
            return;
        }

        auto callback = [cb, id, this](bool ok, const OperationErrorInfo &err, const QString &mpt) {
            // For DVD+RW/DVD-RW discs burned with PW,
            // dfm-burn does not accurately get the capacity information,
            // so it needs to be updated.
            d->watcher->updateOpticalDevUsage(id, mpt);
            Q_EMIT this->blockDevMountResult(id, ok);
            if (ok) {
                qCInfo(logDFMBase) << "Optical device mounted successfully - device:" << id << "mount point:" << mpt;
                Q_EMIT this->blockDevMountedManually(id, mpt);   // redundant: to notify deviceproxymanager update the cache.
            } else {
                qCWarning(logDFMBase) << "Optical device mount failed - device:" << id << "error:" << err.message;
            }
            if (cb)
                cb(ok, err, mpt);
        };

        QFutureWatcher<void> *fw { new QFutureWatcher<void>() };
        connect(fw, &QFutureWatcher<void>::finished, this, [=]() {
            qCDebug(logDFMBase) << "Optical device info query completed, starting mount operation for device:" << id;
            d->isMountingOptical = false;
            dev->mountAsync(options, callback);
            delete fw;
        });
        d->isMountingOptical = true;
        qCInfo(logDFMBase) << "Starting optical device mount process - device:" << id;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        fw->setFuture(QtConcurrent::run(&DeviceWatcher::queryOpticalDevUsage, d->watcher, id));
#else
        fw->setFuture(QtConcurrent::run(d->watcher, &DeviceWatcher::queryOpticalDevUsage, id));
#endif
    } else {
        QString errMsg;
        if (DeviceHelper::isMountableBlockDev(dev, errMsg)) {
            bool removable = dev->removable();
            const auto &cryptoBackingDev = dev->getProperty(Property::kBlockCryptoBackingDevice).toString();
            if (cryptoBackingDev != "/") {
                auto backingDev = DeviceHelper::createBlockDevice(cryptoBackingDev);
                if (!backingDev) {
                    qCWarning(logDFMBase) << "Failed to create backing device for encrypted device:" << id << "backing device:" << cryptoBackingDev;
                    if (cb)
                        cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError), "");
                    Q_EMIT blockDevMountResult(id, false);
                    return;
                }
                removable = backingDev->removable();
            }

            bool optical = dev->optical();
            auto callback = [cb, removable, optical, id, timeout, this](bool ok, const OperationErrorInfo &err, const QString &mpt) {
                this->blockDevMountResult(id, ok);
                if (!mpt.isEmpty() && removable && !optical) {
                    DeviceManagerPrivate::handleDlnfsMount(mpt, true);
                    qCDebug(logDFMBase) << "DLNFS mount handled for removable device:" << id << "mount point:" << mpt;
                }

                if (ok) {
                    qCInfo(logDFMBase) << "Block device mounted successfully - device:" << id << "mount point:" << mpt;
                    Q_EMIT this->blockDevMountedManually(id, mpt);   // redundant: to notify deviceproxymanager update the cache.
                } else {
                    qCWarning(logDFMBase) << "Block device mount failed - device:" << id << "error code:" << err.code << "message:" << err.message;
                }

                if (cb)
                    cb(ok, err, mpt);

                if (mpt.isEmpty() && err.code != DeviceError::kUDisksErrorAlreadyMounted) {
                    qCWarning(logDFMBase) << "Mount operation failed, scheduling retry for device:" << id << "attempt:" << (timeout + 1);
                    retryMount(id, DeviceType::kBlockDevice, timeout + 1);   // if device not mounted, mount it again
                }
            };
            qCInfo(logDFMBase) << "Starting block device mount operation - device:" << id;
            dev->mountAsync(options, callback);
        } else {
            qCWarning(logDFMBase) << "Device is not mountable - device:" << id << "reason:" << errMsg;
            if (cb)
                cb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorNotMountable), "");

            Q_EMIT blockDevMountResult(id, false);
        }
    }
}

bool DeviceManager::unmountBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "Failed to create block device for unmounting:" << id;
        return false;
    }

    auto mpt = dev->mountPoint();
    if (!mpt.isEmpty() && !DeviceHelper::askForStopScanning(QUrl::fromLocalFile(mpt))) {
        qCWarning(logDFMBase) << "Cannot unmount device, scanning operation in progress:" << id << "mount point:" << mpt;
        return false;
    }

    if (dev->isEncrypted()) {
        bool noLock = opts.value(OperateParamField::kUnmountWithoutLock, false).toBool();
        auto options = opts;
        options.remove(OperateParamField::kUnmountWithoutLock);

        QString cleartextId = dev->getProperty(Property::kEncryptedCleartextDevice).toString();
        if (cleartextId == "/") {   // which means it's already unmounted and locked.
            qCDebug(logDFMBase) << "Encrypted device already unmounted and locked:" << id;
            return true;
        }

        qCInfo(logDFMBase) << "Unmounting encrypted device - device:" << id << "cleartext device:" << cleartextId << "lock after unmount:" << !noLock;
        return noLock ? unmountBlockDev(cleartextId, options)
                      : unmountBlockDev(cleartextId, options) && dev->lock();
    } else {
        if (mpt.isEmpty() && dev->mountPoints().isEmpty()) {
            qCDebug(logDFMBase) << "Device is not mounted, unmount operation skipped:" << id;
            return true;
        }
        if (!dev->hasFileSystem()) {
            qCDebug(logDFMBase) << "Device has no filesystem, unmount operation skipped:" << id;
            return true;
        }

        DeviceManagerPrivate::unmountStackedMount(mpt);
        qCInfo(logDFMBase) << "Unmounting block device - device:" << id << "mount point:" << mpt;
        return dev->unmount(opts);
    }
}

void DeviceManager::unmountBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError));
        emit blockDevUnmountAsyncFailed(id, DeviceError::kUnhandledError);
        return;
    }

    auto mpt = dev->mountPoint();
    if (mpt.isEmpty() && !dev->isEncrypted()) {
        if (cb)
            cb(true, Utils::genOperateErrorInfo(DeviceError::kNoError));
        return;
    }

    if (!mpt.isEmpty() && !DeviceHelper::askForStopScanning(QUrl::fromLocalFile(mpt))) {
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUDisksErrorDeviceBusy));
        return;
    }

    if (dev->isEncrypted()) {
        bool noLock = opts.value(OperateParamField::kUnmountWithoutLock, false).toBool();
        auto options = opts;
        options.remove(OperateParamField::kUnmountWithoutLock);

        const QString &cleartextId = dev->getProperty(Property::kEncryptedCleartextDevice).toString();
        if (cleartextId == "/") {
            if (cb)
                cb(true, Utils::genOperateErrorInfo(DeviceError::kNoError));
            return;
        }

        unmountBlockDevAsync(cleartextId, options, [=](bool ok, const OperationErrorInfo &err) {
            qCInfo(logDFMBase) << "unmount device finished: " << id << cleartextId << ok << err.code << err.message;
            if (ok && !noLock)
                dev->lockAsync({});
            else
                emit blockDevUnmountAsyncFailed(id, err.code);
            if (cb)
                cb(ok, err);
        });
    } else {
        DeviceManagerPrivate::unmountStackedMount(mpt);
        dev->unmountAsync(opts, [cb, this, id](bool ok, const OperationErrorInfo &err) {
            qCInfo(logDFMBase) << "unmount device finished: " << id << ok << err.code << err.message;
            if (cb)
                cb(ok, err);
            if (!ok)
                emit blockDevUnmountAsyncFailed(id, err.code);
        });
    }
}

void DeviceManager::lockBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError));
        return;
    }

    if (!dev->isEncrypted()) {
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorNotEncryptable));
    }

    dev->lockAsync(opts, cb);
}

void DeviceManager::unlockBlockDevAsync(const QString &id, const QString &passwd, const QVariantMap &opts, CallbackType1 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError), "");
        return;
    }

    if (!dev->isEncrypted()) {
        qCWarning(logDFMBase) << "this is not a lockable device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorNotEncryptable), "");
        return;
    }

    QString cleartextId = dev->getProperty(Property::kEncryptedCleartextDevice).toString();
    if (cleartextId != "/") {
        if (cb)
            cb(true, Utils::genOperateErrorInfo(DeviceError::kNoError), cleartextId);
        return;
    }
    dev->unlockAsync(passwd, opts, cb);
}

void DeviceManager::powerOffBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError));
        emit blockDevPoweroffAysncFailed(id, DeviceError::kUnhandledError);
        return;
    }

    if (!dev->canPowerOff()) {
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorNotPoweroffable));
        emit blockDevPoweroffAysncFailed(id, DeviceError::kUserErrorNotPoweroffable);
        return;
    }

    dev->powerOffAsync(opts, [this, cb, id](bool ok, const OperationErrorInfo &err) {
        qCInfo(logDFMBase) << "power off device finished: " << ok << err.code << err.message;
        if (cb)
            cb(ok, err);
        if (!ok)
            emit blockDevPoweroffAysncFailed(id, err.code);
    });
}

void DeviceManager::ejectBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError));
        emit blockDevEjectAsyncFailed(id, DeviceError::kUnhandledError);
        return;
    }

    QString errMsg;
    if (!DeviceHelper::isEjectableBlockDev(dev, errMsg)) {
        qCWarning(logDFMBase) << "device cannot be eject!" << errMsg;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorNotEjectable));
        emit blockDevEjectAsyncFailed(id, DeviceError::kUserErrorNotEjectable);
        return;
    }
    dev->ejectAsync(opts, [this, id, cb](bool ok, const OperationErrorInfo &err) {
        qCInfo(logDFMBase) << "eject device finished: " << ok << err.code << err.message;
        if (cb)
            cb(ok, err);
        if (!ok)
            emit blockDevEjectAsyncFailed(id, err.code);
    });
}

void DeviceManager::renameBlockDevAsync(const QString &id, const QString &newName, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError));
        return;
    }

    if (dev->isEncrypted()) {
        auto clearDevId = dev->getProperty(Property::kEncryptedCleartextDevice).toString();
        if (clearDevId == "/") {   // the unlockded device's cleartext device's id is '/'
            qCInfo(logDFMBase) << "rename: locked device cannot be renamed: " << id;
            return;
        }
        qCDebug(logDFMBase) << "rename: redirect to cleartext device: " << clearDevId;
        renameBlockDevAsync(clearDevId, newName, opts, cb);
        return;
    }

    if (!dev->hasFileSystem()) {
        qCWarning(logDFMBase) << "device cannot be renames cause it does not have filesystem interface" << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorNotMountable));
        return;
    }

    if (!dev->mountPoint().isEmpty()) {
        qCWarning(logDFMBase) << "device cannot be renamed cause it's still mounted yet" << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorAlreadyMounted));
        return;
    }

    dev->renameAsync(newName, opts, cb);
}

bool DeviceManager::rescanBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        return false;
    }

    return dev->rescan(opts);
}

void DeviceManager::rescanBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError));
        return;
    }
    dev->rescanAsync(opts, cb);
}

QString DeviceManager::mountProtocolDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        return "";
    }
    return dev->mount(opts);
}

void DeviceManager::mountProtocolDevAsync(const QString &id, const QVariantMap &opts, CallbackType1 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError), "");
        return;
    }
    dev->mountAsync(opts, cb);
}

bool DeviceManager::unmountProtocolDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        return false;
    }
    return dev->unmount(opts);
}

void DeviceManager::unmountProtocolDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "cannot create block device: " << id;
        if (cb)
            cb(false, Utils::genOperateErrorInfo(DeviceError::kUnhandledError));
        return;
    }
    dev->unmountAsync(opts, cb);
}

/*!
 * \brief DeviceManager::mountNetworkDeviceAsync
 * \param address: like smb://1.2.3.4/HelloWorld
 * \param cb: callback when mount finished.
 * \param timeout: seconds.
 */
void DeviceManager::mountNetworkDeviceAsync(const QString &address, CallbackType1 cb, int timeout)
{
    //    Q_ASSERT(qApp->thread() == QThread::currentThread());
    Q_ASSERT_X(!address.isEmpty(), __FUNCTION__, "address is emtpy");
    QUrl u(address);
    if (!u.isValid()) {
        qCWarning(logDFMBase) << "url is not valid: " << u << address;
        return;
    }

    static QMap<QString, QString> defaultPort { { "smb", "445" },
                                                { "ftp", "21" },
                                                { "sftp", "22" },
                                                { "nfs", "2049" } };
    QString host = u.host();
    QString port = defaultPort.value(u.scheme());

    static QRegularExpression regUrl(R"((\w+)://([^/:]+)(:\d*)?)");
    auto match = regUrl.match(address);
    if (match.hasMatch()) {
        auto capPort = match.captured(3).mid(1);   // remove first ':'
        if (!capPort.isEmpty())
            port = capPort;
    }

    using namespace std::placeholders;
    auto func = std::bind(DeviceManagerPrivate::askForPasswdWhenMountNetworkDevice, _1, _2, _3, address);

    auto wrappedCb = [=](bool ok, const OperationErrorInfo &err, const QString &msg) {
        Q_EMIT mountNetworkDeviceResult(address, ok, err.code, err.code == DeviceError::kNoError ? msg : err.message);
        if (cb) cb(ok, err, msg);
        QApplication::restoreOverrideCursor();
    };

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QStringList ports;
    if (!port.isEmpty())
        ports.append(port);
    static const QStringList &defaultSmbPorts { "445", "139" };
    if (u.scheme() == "smb" && defaultSmbPorts.contains(port))
        ports = defaultSmbPorts;

    NetworkUtils::instance()->doAfterCheckNet(host, ports, [=](bool ok) {
        QApplication::restoreOverrideCursor();
        if (ok) {
            DProtocolDevice::mountNetworkDevice(address, func, DeviceManagerPrivate::askForUserChoice,
                                                wrappedCb, timeout);
        } else {
            wrappedCb(false, Utils::genOperateErrorInfo(DeviceError::kUserErrorTimedOut), "");
            qCWarning(logDFMBase) << "cannot access network " << host << ":" << port;
        }
    });
}

void DeviceManager::doAutoMountAtStart()
{
    if (!DeviceUtils::isAutoMountEnable()) {
        qCInfo(logDFMBase) << "Auto mount is disabled in system settings";
        return;
    }

    if (UniversalUtils::currentLoginUser() != getuid()) {
        qCInfo(logDFMBase) << "Auto mount skipped - current user is not the logged-in user";
        return;
    }

    static std::once_flag flg;
    std::call_once(flg, [this] {
        qCInfo(logDFMBase) << "Starting auto mount process at application startup";
        d->mountAllBlockDev();
    });
}

void DeviceManager::detachAllRemovableBlockDevs()
{
    const QStringList &&devs = getAllBlockDevID(DeviceQueryOption::kMounted | DeviceQueryOption::kRemovable);
    // NOTE(xust): since the `Removable` is not always correct in some certain hardwares,
    // we should ignore those devices which cannot be poweroffed, and is not optical item.
    QStringList filteredDevs = devs;
    for (const QString &id : devs) {
        auto info = DeviceHelper::loadBlockInfo(id);
        if (!info.value(DeviceProperty::kCanPowerOff).toBool()
            && !info.value(DeviceProperty::kOptical).toBool())
            filteredDevs.removeAll(id);
        if (DeviceUtils::isSiblingOfRoot(info))
            filteredDevs.removeAll(id);
    }

    qCInfo(logDFMBase) << "about to detaching" << filteredDevs;

    QStringList operated;
    for (const auto &id : filteredDevs) {
        if (operated.contains(id))
            continue;
        operated << detachBlockDev(id);
    }
}

/*!
 * \brief DeviceManager::detachBlockDev
 * \param id
 * \param cb
 * \return
 * this function will unmount all associated block devices, and returns the id list of the unmounted
 * devices.
 */
QStringList DeviceManager::detachBlockDev(const QString &id, CallbackType2 cb)
{
    auto siblings = d->watcher->getSiblings(id);
    qCDebug(logDFMBase) << "got siblings of " << id << "\n"
                        << siblings;
    if (siblings.isEmpty())
        siblings << id;
    const auto &&me = DeviceHelper::loadBlockInfo(id);
    bool isOptical = me.value(DeviceProperty::kOpticalDrive).toBool();
    bool canPowerOff = me.value(DeviceProperty::kCanPowerOff).toBool();

    auto media = me.value(DeviceProperty::kMedia).toString();
    auto func = [this, id, isOptical, canPowerOff, cb, media](bool allUnmounted, const OperationErrorInfo &err) {
        if (allUnmounted) {
            QThread::msleep(500);   // make a short delay to eject/powerOff, other wise may raise a
                                    // 'device busy' error.
            if (isOptical)
                ejectBlockDevAsync(id, {}, cb);
            else if (canPowerOff && media != "flash_sd")   // do not detach SD driver.
                powerOffBlockDevAsync(id, {}, cb);
            else if (cb)
                cb(true, err);
        } else {
            if (cb)
                cb(false, err);
        }
    };

    QSharedPointer<bool> allUnmounted(new bool(true));
    QSharedPointer<int> opCount(new int(siblings.count()));
    for (const auto &dev : siblings) {
        unmountBlockDevAsync(dev, {}, [allUnmounted, func, opCount, dev](bool ok, const OperationErrorInfo &err) {
            *allUnmounted &= ok;
            *opCount -= 1;
            qCInfo(logDFMBase) << "detach device: " << dev << ", siblings remain: " << *opCount
                               << ", success? " << ok << err.message;
            if (*opCount == 0)
                func(*allUnmounted, err);
        });
    }

    return siblings;
}

void DeviceManager::detachAllProtoDevs()
{
    const QStringList &&devs = getAllProtocolDevID();
    for (const auto &id : devs)
        detachProtoDev(id);
}

void DeviceManager::detachProtoDev(const QString &id)
{
    unmountProtocolDevAsync(id, {}, [this, id](bool ok, const OperationErrorInfo &err) {
        if (!ok) {
            qCWarning(logDFMBase) << "unmount protocol device failed: " << id << err.message << err.code;
            emit protocolDevUnmountAsyncFailed(id, err.code);
        }
    });
}

void DeviceManager::startPollingDeviceUsage()
{
    d->watcher->startPollingUsage();
}

void DeviceManager::stopPollingDeviceUsage()
{
    d->watcher->stopPollingUsage();
}

void DeviceManager::startMonitor()
{
    if (isMonitoring())
        return;
    d->watcher->startWatch();
    d->watcher->initDevDatas();
    d->isWatching = true;
}

void DeviceManager::stopMonitor()
{
    d->watcher->stopWatch();
    d->isWatching = false;
}

bool DeviceManager::isMonitoring()
{
    return d->isWatching;
}

void DeviceManager::startOpticalDiscScan()
{
    d->discScanner->initialize();
}

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent), d(new DeviceManagerPrivate(this))
{
}

DeviceManager::~DeviceManager() { }

void DeviceManager::doAutoMount(const QString &id, DeviceType type, int timeout)
{
    if (type == DeviceType::kProtocolDevice) {   // alwasy auto mount protocol device
        qCDebug(logDFMBase) << "Auto mounting protocol device:" << id;
        mountProtocolDevAsync(id);
        return;
    }

    if (!d->autoMountBlock) {
        qCDebug(logDFMBase) << "Auto mount for block devices is disabled in current application context";
        return;
    }

    if (!DeviceUtils::isAutoMountEnable()) {
        qCDebug(logDFMBase) << "Auto mount is disabled in system settings";
        return;
    }
    if (!UniversalUtils::isLogined()) {
        qCDebug(logDFMBase) << "Auto mount skipped - no user logged in for device:" << id;
        return;
    }

    if (UniversalUtils::currentLoginUser() != getuid()) {
        qCDebug(logDFMBase) << "Auto mount skipped - current user is not the logged-in user for device:" << id;
        return;
    }

    if (UniversalUtils::isInLiveSys()) {
        qCDebug(logDFMBase) << "Auto mount disabled in live system for device:" << id;
        return;
    }

    CallbackType1 cb = nullptr;
    if (DeviceUtils::isAutoMountAndOpenEnable()) {
        cb = [id](bool ok, const OperationErrorInfo &, const QString &mpt) {
            if (ok) {
                qCInfo(logDFMBase) << "Auto mount and open - opening file manager for device:" << id << "mount point:" << mpt;
                DeviceHelper::openFileManagerToDevice(id, mpt);
            }
        };
    }

    if (type == DeviceType::kBlockDevice) {
        auto &&info = getBlockDevInfo(id);
        if (info.value(DeviceProperty::kIsEncrypted).toBool()
            || info.value(DeviceProperty::kCryptoBackingDevice).toString() != "/") {
            qCDebug(logDFMBase) << "Auto mount skipped for encrypted device:" << id;
            return;
        }
        if (info.value(DeviceProperty::kHintIgnore).toBool()) {
            qCDebug(logDFMBase) << "Auto mount skipped for ignored device:" << id;
            return;
        }
        if (!info.value(DeviceProperty::kHasFileSystem).toBool()) {
            qCDebug(logDFMBase) << "Auto mount skipped for device without filesystem:" << id;
            return;
        }

        qCInfo(logDFMBase) << "Starting auto mount for block device:" << id;
        mountBlockDevAsync(id, {}, cb, timeout);
    }
}

void DeviceManager::retryMount(const QString &id, DFMMOUNT::DeviceType type, int timeout)
{
    if (timeout > 1) {
        qCWarning(logDFMBase) << "Mount retry limit exceeded for device:" << id << "attempts:" << timeout;
        return;
    }

    qCInfo(logDFMBase) << "Scheduling mount retry in 5 seconds for device:" << id << "attempt:" << timeout;
    QTimer::singleShot(5000, this, [id, type, timeout] { DeviceManager::instance()->doAutoMount(id, type, timeout); });
}

DeviceManagerPrivate::DeviceManagerPrivate(DeviceManager *qq)
    : watcher(new DeviceWatcher(qq)), discScanner(new DiscDeviceScanner(qq)), q(qq)
{
}

void DeviceManagerPrivate::mountAllBlockDev()
{
    const QStringList &devs { q->getAllBlockDevID(DeviceQueryOption::kMountable | DeviceQueryOption::kNotIgnored
                                                  | DeviceQueryOption::kNotMounted) };
    qCInfo(logDFMBase) << "Starting auto mount for" << devs.size() << "mountable block devices:" << devs;
    for (const auto &dev : devs) {
        if (dev.startsWith("/org/freedesktop/UDisks2/block_devices/sr")) {
            qCDebug(logDFMBase) << "Skipping auto mount for optical device:" << dev;
            continue;
        }
        q->mountBlockDevAsync(dev, { { "auth.no_user_interaction", true } });   // avoid the auth dialog raising
    }
}

bool DeviceManagerPrivate::isDaemonMountRunning()
{
    auto systemBusIFace = QDBusConnection::systemBus().interface();
    if (!systemBusIFace) {
        qCWarning(logDFMBase) << "daemon mount is not available.";
        return false;
    }

    if (!systemBusIFace->isServiceRegistered(kDaemonService)) {
        qCWarning(logDFMBase) << "daemon service is not registered";
        return false;
    }

    QDBusInterface daemonIface(kDaemonService, kDaemonPath, kDaemonIntroIface,
                               QDBusConnection::systemBus());
    QDBusReply<QString> reply = daemonIface.call(kDaemonIntroMethod);
    qCDebug(logDFMBase) << reply.value();
    return reply.value().contains("<node name=\"MountControl\"/>");
}

/*!
 * \brief DeviceManagerPrivate::handleDlnfsMount
 * \param mpt
 * \param mount: TRUE for mount, FALSE for unmount
 */
void DeviceManagerPrivate::handleDlnfsMount(const QString &mpt, bool mount)
{
    if (mount) {
        auto enableDlnfsMount = DConfigManager::instance()->value(kDefaultCfgPath, "dfm.mount.dlnfs").toBool();
        if (!enableDlnfsMount) {
            qCDebug(logDFMBase) << "DLNFS mount is disabled in configuration";
            return;
        }
    }

    // Check if DLNFS service is available
    QDBusConnectionInterface *systemBusIFace = QDBusConnection::systemBus().interface();
    if (!systemBusIFace->isServiceRegistered(kDlnfsService)) {
        qCWarning(logDFMBase) << "DLNFS service is not available:" << kDlnfsService;
        return;
    }

    QString method = mount ? "Mount" : "Unmount";

    qCInfo(logDFMBase) << "Starting DLNFS" << method.toLower() << "operation for mount point:" << mpt;

    QDBusInterface iface(kDlnfsService, kDlnfsPath, kDlnfsIface, QDBusConnection::systemBus());
    QDBusReply<QVariantMap> reply = iface.call(method, mpt, QVariantMap { { "fsType", "dlnfs" } });
    const auto &ret = reply.value();

    if (ret.value("result").toBool()) {
        qCInfo(logDFMBase) << "DLNFS" << method.toLower() << "operation completed successfully for mount point:" << mpt;
    } else {
        qCWarning(logDFMBase) << "DLNFS" << method.toLower() << "operation failed for mount point:" << mpt << "result:" << ret;
    }
}

void DeviceManagerPrivate::unmountStackedMount(const QString &mpt)
{
    // TODO: Not sure what this code does, will look at it when something goes wrong
#if 0
    QDBusInterface iface(kDaemonService, kDaemonMountPath, kDaemonMountIface, QDBusConnection::systemBus());
    QDBusReply<QVariantMap> reply = iface.call("Unmount", mpt,
                                               QVariantMap { { "fsType", "common" }, { "unmountAllStacked", true } });
    const auto &ret = reply.value();
    qCDebug(logDFMBase) << "unmount all stacked mount of: " << mpt << ret;
#endif
}

MountPassInfo DeviceManagerPrivate::askForPasswdWhenMountNetworkDevice(const QString &message, const QString &userDefault,
                                                                       const QString &domainDefault, const QString &uri)
{
    MountAskPasswordDialog dlg(qApp->activeWindow());

    // daemon mount return plain text which should be replaced with translated text.
    QString msg(message);
    static const QString kCustomMessagePrefix = "need authorization to access";
    if (msg.startsWith(kCustomMessagePrefix))
        msg.replace(kCustomMessagePrefix, QObject::tr("need authorization to access"));
    static const QString kAuthMessagePrefix = "Authentication Required\nEnter user and password for";
    if (msg.startsWith(kAuthMessagePrefix)) {
        QString hostOrIp = msg.mid(kAuthMessagePrefix.length()).trimmed();
        // 去除末尾冒号
        if (hostOrIp.endsWith(':') || hostOrIp.endsWith("："))
            hostOrIp.chop(1);
        msg = QObject::tr("Authentication Required\nEnter user and password for %1").arg(hostOrIp) + QString("：");
    }

    dlg.setTitle(msg);
    dlg.setDomain(domainDefault);
    dlg.setUser(userDefault);

    QStringList noDomainSchemes { Global::Scheme::kSFtp, Global::Scheme::kFtp, Global::Scheme::kDav, Global::Scheme::kDavs };
    if (noDomainSchemes.contains(QUrl(uri).scheme()))
        dlg.setDomainLineVisible(false);

    DFMMOUNT::MountPassInfo info;
    QApplication::restoreOverrideCursor();

    // 将对话框手动关闭，以及用户取消输入都作为取消操作
    if (dlg.exec() != QDialog::Accepted) {
        info.cancelled = true;
        QApplication::setOverrideCursor(Qt::WaitCursor);
        return info;
    }

    QJsonObject loginInfo = dlg.getLoginData();
    auto data = loginInfo.toVariantMap();
    using namespace GlobalServerDefines::NetworkMountParamKey;
    if (data.value(kAnonymous, false).toBool()) {
        info.anonymous = true;
    } else {
        info.userName = data.value(kUser).toString();
        info.domain = data.value(kDomain).toString();
        info.savePasswd = static_cast<DFMMOUNT::NetworkMountPasswdSaveMode>(data.value(kPasswdSaveMode).toInt());
        QString pwd = data.value(kPasswd).toString();
        info.passwd = DProtocolDevice::isMountByDaemon(uri) ? encryptPasswd(pwd) : pwd;

        // save password in session if samba is integrated.
        if (uri.startsWith(Global::Scheme::kSmb)) {
            using AppGA = Application::GenericAttribute;
            bool smbIntegrated = Application::genericAttribute(AppGA::kMergeTheEntriesOfSambaSharedFolders).toBool();
            if (smbIntegrated && info.savePasswd != dfmmount::NetworkMountPasswdSaveMode::kSavePermanently)
                info.savePasswd = NetworkMountPasswdSaveMode::kSaveBeforeLogout;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    return info;
}

QString DeviceManagerPrivate::encryptPasswd(const QString &passwd)
{
    // todo use encrypt-plugin
    QByteArray byteArray = passwd.toUtf8();
    QByteArray encodedByteArray = byteArray.toBase64();
    return QString::fromUtf8(encodedByteArray);
}

int DeviceManagerPrivate::askForUserChoice(const QString &message, const QStringList &choices)
{
    QString newMsg = message;
    QString title;
    if (message.startsWith("Can't verify the identity of")
        /*&& message.endsWith("If you want to be absolutely sure it is safe to continue, contact the "
                            "system administrator.")*/
    ) {
        QString arg1, arg2;
        QRegularExpression reg("“.*?”");
        auto matcher = reg.match(message);
        if (matcher.hasMatch()) {
            arg1 = matcher.captured(0);
            newMsg = newMsg.replace(arg1, "");

            matcher = reg.match(newMsg);
            arg2 = matcher.captured(0);

            title = QObject::tr("Can't verify the identity of %1.").arg(arg1);
            newMsg = QObject::tr("This happens when you log in to a computer the first time.")
                    + '\n' + QObject::tr("The identity sent by the remote computer is") + '\n'
                    + arg2 + '\n'
                    + QObject::tr("If you want to be absolutely sure it is safe to continue, "
                                  "contact the system administrator.");
        }
        newMsg = newMsg.replace("\\r\\n", "\n");

        qCDebug(logDFMBase) << "filtered question message is: " << newMsg;
    }
    DWIDGET_USE_NAMESPACE
    DDialog askForChoice(qApp->activeWindow());
    askForChoice.setTitle(title);
    askForChoice.setMessage(newMsg);
    askForChoice.addButtons(choices);

    return askForChoice.exec();
}
