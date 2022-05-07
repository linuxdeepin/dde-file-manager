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
#include "devicemanager.h"
#include "deviceutils.h"
#include "private/devicemanager_p.h"
#include "private/devicehelper.h"
#include "private/devicewatcher.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include <dfm-mount/dfm-mount.h>
#include <mutex>

DFMBASE_USE_NAMESPACE
DFM_MOUNT_USE_NS
using namespace GlobalServerDefines;

QStringList DeviceManager::getAllBlockDevID(DeviceQueryOptions opts)
{
    auto &&ret = d->watcher->getDevIds(DeviceType::BlockDevice);
    ret.sort();
    if (opts.testFlag(DeviceQueryOption::kNoCondition))
        return ret;

    QString errMsg;
    QStringList filteredRet;
    for (const auto &id : ret) {
        const auto &&data = d->watcher->getDevInfo(id, DeviceType::BlockDevice, false);
        if (opts.testFlag(DeviceQueryOption::kMounted) && data.value(DeviceProperty::kMountPoint).toString().isEmpty())
            continue;
        if (opts.testFlag(DeviceQueryOption::kRemovable) && !data.value(DeviceProperty::kRemovable).toBool())
            continue;
        if (opts.testFlag(DeviceQueryOption::kMountable) && !DeviceHelper::isMountableBlockDev(data, errMsg))
            continue;
        if (opts.testFlag(DeviceQueryOption::kNotIgnored) && data.value(DeviceProperty::kHintIgnore).toBool())
            continue;
        if (opts.testFlag(DeviceQueryOption::kNotMounted) && !data.value(DeviceProperty::kMountPoint).toString().isEmpty())
            continue;
        if (opts.testFlag(DeviceQueryOption::kOptical) && !data.value(DeviceProperty::kOptical).toBool())
            continue;
        filteredRet << id;
    }
    return filteredRet;
}

QStringList DeviceManager::getAllProtocolDevID()
{
    auto &&ret = d->watcher->getDevIds(DeviceType::ProtocolDevice);
    ret.sort();
    return ret;
}

QVariantMap DeviceManager::getBlockDevInfo(const QString &id, bool needReload)
{
    return d->watcher->getDevInfo(id, DeviceType::BlockDevice, needReload);
}

QVariantMap DeviceManager::getProtocolDevInfo(const QString &id, bool needReload)
{
    return d->watcher->getDevInfo(id, DeviceType::ProtocolDevice, needReload);
}

QString DeviceManager::mountBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return "";
    }

    if (dev->optical())
        d->watcher->queryOpticalDevUsage(id);

    QString errMsg;
    if (DeviceHelper::isMountableBlockDev(dev, errMsg)) {
        return dev->mount(opts);
    } else {
        // TODO, handle optical
        qWarning() << "device is not mountable: " << errMsg << id;
        return "";
    }
}

void DeviceManager::mountBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType1 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError, "");
        return;
    }

    if (dev->optical()) {
        QFutureWatcher<void> *fw { new QFutureWatcher<void>() };
        connect(fw, &QFutureWatcher<void>::finished, this, [=]() {
            dev->mountAsync(opts, cb);
            delete fw;
        });
        fw->setFuture(QtConcurrent::run(d->watcher, &DeviceWatcher::queryOpticalDevUsage, id));
    } else {
        QString errMsg;
        if (DeviceHelper::isMountableBlockDev(dev, errMsg)) {
            dev->mountAsync(opts, cb);
        } else {
            qWarning() << "device is not mountable: " << errMsg << id;
            if (cb)
                cb(false, DeviceError::UserErrorNotMountable, "");
        }
    }
}

bool DeviceManager::unmountBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return false;
    }

    auto mpt = dev->mountPoint();
    if (!mpt.isEmpty() && !DeviceHelper::askForStopScanning(QUrl::fromLocalFile(mpt)))
        return false;

    if (dev->isEncrypted()) {
        QString cleartextId = dev->getProperty(Property::EncryptedCleartextDevice).toString();
        if (cleartextId == "/")   // which means it's already unmounted and locked.
            return true;
        return unmountBlockDev(cleartextId, opts) && dev->lock();   // otherwise unmount the decrypted device and lock the shell device.
    } else {
        if (mpt.isEmpty() && dev->mountPoints().isEmpty())
            return true;
        if (!dev->hasFileSystem())
            return true;
        return dev->unmount(opts);
    }
}

void DeviceManager::unmountBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError);
        emit blockDevUnmountAsyncFailed(id, DeviceError::UnhandledError);
        return;
    }

    auto mpt = dev->mountPoint();
    if (!mpt.isEmpty() && !DeviceHelper::askForStopScanning(QUrl::fromLocalFile(mpt)))
        return;

    if (dev->isEncrypted()) {
        const QString &cleartextId = dev->getProperty(Property::EncryptedCleartextDevice).toString();
        if (cleartextId == "/") {
            if (cb)
                cb(true, DeviceError::NoError);
            return;
        }

        unmountBlockDevAsync(cleartextId, opts, [dev, cb, id, this](bool ok, DeviceError err) {
            if (ok)
                dev->lockAsync({});
            else
                emit blockDevUnmountAsyncFailed(id, err);
            if (cb)
                cb(ok, err);
        });
    } else {
        dev->unmountAsync(opts, [cb, this, id](bool ok, DeviceError err) {
            if (cb)
                cb(ok, err);
            if (!ok)
                emit blockDevUnmountAsyncFailed(id, err);
        });
    }
}

bool DeviceManager::lockBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return false;
    }

    if (!dev->isEncrypted()) {
        qWarning() << "this is not a lockable device: " << id;
        return false;
    }

    if (dev->lock(opts)) {
        return true;
    } else {
        qWarning() << "lock device failed: " << DeviceUtils::errMessage(dev->lastError());
        return false;
    }
}

void DeviceManager::lockBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError);
        return;
    }

    if (!dev->isEncrypted()) {
        if (cb)
            cb(false, DeviceError::UserErrorNotEncryptable);
    }

    dev->lockAsync(opts, cb);
}

QString DeviceManager::unlockBlockDev(const QString &id, const QString &passwd, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return "";
    }

    if (!dev->isEncrypted()) {
        qWarning() << "this is not a lockable device: " << id;
        return "";
    }

    QString cleartextId = dev->getProperty(Property::EncryptedCleartextDevice).toString();
    if (cleartextId != "/")
        return cleartextId;

    cleartextId.clear();
    bool ok = dev->unlock(passwd, cleartextId, opts);
    if (!ok)
        qWarning() << "unlock device failed: " << id << DeviceUtils::errMessage(dev->lastError());
    return cleartextId;
}

void DeviceManager::unlockBlockDevAsync(const QString &id, const QString &passwd, const QVariantMap &opts, CallbackType1 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError, "");
        return;
    }

    if (!dev->isEncrypted()) {
        qWarning() << "this is not a lockable device: " << id;
        if (cb)
            cb(false, DeviceError::UserErrorNotEncryptable, "");
        return;
    }

    QString cleartextId = dev->getProperty(Property::EncryptedCleartextDevice).toString();
    if (cleartextId != "/") {
        if (cb)
            cb(true, DeviceError::NoError, cleartextId);
        return;
    }
    dev->unlockAsync(passwd, opts, cb);
}

bool DeviceManager::powerOffBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return false;
    }

    if (!dev->canPowerOff()) {
        qWarning() << "device cannot be poweroff! " << id;
        return false;
    }
    return dev->powerOff(opts);
}

void DeviceManager::powerOffBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError);
        emit blockDevPoweroffAysncFailed(id, DeviceError::UnhandledError);
        return;
    }

    if (!dev->canPowerOff()) {
        if (cb)
            cb(false, DeviceError::UserErrorNotPoweroffable);
        emit blockDevPoweroffAysncFailed(id, DeviceError::UserErrorNotPoweroffable);
        return;
    }

    dev->powerOffAsync(opts, [this, cb, id](bool ok, DeviceError err) {
        if (cb)
            cb(ok, err);
        if (!ok)
            emit blockDevPoweroffAysncFailed(id, err);
    });
}

bool DeviceManager::ejectBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return false;
    }

    QString errMsg;
    if (!DeviceHelper::isEjectableBlockDev(dev, errMsg)) {
        qWarning() << "device cannot be eject!" << errMsg;
        return false;
    }
    return dev->eject(opts);
}

void DeviceManager::ejectBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError);
        emit blockDevEjectAsyncFailed(id, DeviceError::UnhandledError);
        return;
    }

    QString errMsg;
    if (!DeviceHelper::isEjectableBlockDev(dev, errMsg)) {
        qWarning() << "device cannot be eject!" << errMsg;
        if (cb)
            cb(false, DeviceError::UserErrorNotEjectable);
        emit blockDevEjectAsyncFailed(id, DeviceError::UserErrorNotEjectable);
        return;
    }
    dev->ejectAsync(opts, [this, id, cb](bool ok, DeviceError err) {
        if (cb)
            cb(ok, err);
        if (!ok)
            emit blockDevEjectAsyncFailed(id, err);
    });
}

bool DeviceManager::renameBlockDev(const QString &id, const QString &newName, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return false;
    }

    if (!dev->hasFileSystem()) {
        qWarning() << "device cannot be renames cause it does not have filesystem interface" << id;
        return false;
    }

    if (!dev->mountPoint().isEmpty()) {
        qWarning() << "device cannot be renamed cause it's still mounted yet" << id;
        return false;
    }

    return dev->rename(newName, opts);
}

void DeviceManager::renameBlockDevAsync(const QString &id, const QString &newName, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError);
        return;
    }

    if (!dev->hasFileSystem()) {
        qWarning() << "device cannot be renames cause it does not have filesystem interface" << id;
        if (cb)
            cb(false, DeviceError::UserErrorNotMountable);
        return;
    }

    if (!dev->mountPoint().isEmpty()) {
        qWarning() << "device cannot be renamed cause it's still mounted yet" << id;
        if (cb)
            cb(false, DeviceError::UserErrorAlreadyMounted);
        return;
    }

    dev->renameAsync(newName, opts, cb);
}

bool DeviceManager::rescanBlockDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return false;
    }

    return dev->rescan(opts);
}

void DeviceManager::rescanBlockDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError);
        return;
    }
    dev->rescanAsync(opts, cb);
}

QString DeviceManager::mountProtocolDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return "";
    }
    return dev->mount(opts);
}

void DeviceManager::mountProtocolDevAsync(const QString &id, const QVariantMap &opts, CallbackType1 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError, "");
        return;
    }
    dev->mountAsync(opts, cb);
}

bool DeviceManager::unmountProtocolDev(const QString &id, const QVariantMap &opts)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        return false;
    }
    return dev->unmount(opts);
}

void DeviceManager::unmountProtocolDevAsync(const QString &id, const QVariantMap &opts, CallbackType2 cb)
{
    Q_ASSERT_X(!id.isEmpty(), __FUNCTION__, "device id cannot be emtpy!!!");

    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qWarning() << "cannot create block device: " << id;
        if (cb)
            cb(false, DeviceError::UnhandledError);
        return;
    }
    dev->unmountAsync(opts, cb);
}

void DeviceManager::mountNetworkDeviceAsync(const QString &address, CallbackType1 cb, int timeout)
{
    Q_ASSERT_X(!address.isEmpty(), __FUNCTION__, "address is emtpy");

    using namespace std::placeholders;
    auto func = std::bind(DeviceManagerPrivate::askForPasswdWhenMountNetworkDevice, _1, _2, _3, address);
    DFMProtocolDevice::mountNetworkDevice(address, func, DeviceManagerPrivate::askForUserChoice, cb, timeout);
}

void DeviceManager::doAutoMountAtStart()
{
    if (!DeviceUtils::isAutoMountEnable())
        return;

    static std::once_flag flg;
    std::call_once(flg, &DeviceManagerPrivate::mountAllBlockDev, d);
}

void DeviceManager::detachAllRemovableBlockDevs()
{
    const QStringList &&devs = getAllBlockDevID(DeviceQueryOption::kMounted | DeviceQueryOption::kRemovable);
    QStringList operated;
    for (const auto &id : devs) {
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
 * this function will unmount all associated block devices, and returns the id list of the unmounted devices.
 */
QStringList DeviceManager::detachBlockDev(const QString &id, CallbackType2 cb)
{
    auto siblings = d->watcher->getSiblings(id);
    qDebug() << "got siblings of " << id << "\n"
             << siblings;
    if (siblings.isEmpty())
        siblings << id;
    const auto &&me = DeviceHelper::loadBlockInfo(id);
    bool isOptical = me.value(DeviceProperty::kOpticalDrive).toBool();

    auto func = [this, id, isOptical, cb](bool allUnmounted) {
        if (allUnmounted) {
            if (isOptical)
                ejectBlockDevAsync(id, {}, cb);
            else
                powerOffBlockDevAsync(id, {}, cb);
        } else {
            if (cb)
                cb(false, DeviceError::UserErrorFailed);
        }
    };

    bool *allUnmounted = new bool;
    *allUnmounted = true;
    int *opCount = new int;
    *opCount = siblings.count();
    for (const auto &dev : siblings) {
        unmountBlockDevAsync(dev, {}, [allUnmounted, func, opCount, dev](bool ok, DeviceError err) {
            *allUnmounted &= ok;
            *opCount -= 1;
            qDebug() << "detach device: " << dev << ", siblings last: " << *opCount << ", current result: " << ok << DeviceUtils::errMessage(err);
            if (*opCount == 0) {
                func(*allUnmounted);
                delete opCount;
                delete allUnmounted;
            }
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
    unmountProtocolDevAsync(id, {}, [id](bool ok, DeviceError err) {
        if (!ok) {
            qWarning() << "unmount protocol device: " << id;
            qWarning() << "unmount protocol device failed: " << DeviceUtils::errMessage(err);
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

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent), d(new DeviceManagerPrivate(this))
{
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::doAutoMount(const QString &id, DeviceType type)
{
    if (!DeviceUtils::isAutoMountEnable()) {
        qDebug() << "auto mount is disabled";
        return;
    }
    if (!UniversalUtils::isLogined()) {
        qDebug() << "no login no auto mount";
        return;
    }
    if (UniversalUtils::isInLiveSys()) {
        qDebug() << "no auto mount in live system";
        return;
    }

    CallbackType1 cb = nullptr;
    if (DeviceUtils::isAutoMountAndOpenEnable()) {
        cb = [id](bool ok, DeviceError err, const QString &mpt) {
            if (ok)
                DeviceHelper::openFileManagerToDevice(id, mpt);
        };
    }

    if (type == DeviceType::BlockDevice)
        mountBlockDevAsync(id, {}, cb);
    else if (type == DeviceType::ProtocolDevice)
        mountProtocolDevAsync(id);
}

DeviceManagerPrivate::DeviceManagerPrivate(DeviceManager *qq)
    : watcher(new DeviceWatcher(qq)), q(qq)
{
}

void DeviceManagerPrivate::mountAllBlockDev()
{
    const QStringList &devs { q->getAllBlockDevID(DeviceQueryOption::kRemovable | DeviceQueryOption::kMountable
                                                  | DeviceQueryOption::kNotIgnored | DeviceQueryOption::kNotMounted) };
    qDebug() << "start to mount block devs: " << devs;
    for (const auto &dev : devs)
        q->mountBlockDevAsync(dev);
}

MountPassInfo DeviceManagerPrivate::askForPasswdWhenMountNetworkDevice(const QString &message, const QString &userDefault, const QString &domainDefault, const QString &uri)
{
    MountAskPasswordDialog dlg;
    dlg.setTitle(message);
    dlg.setDomain(domainDefault);
    dlg.setUser(userDefault);

    if (uri.startsWith(Global::kFtp) || uri.startsWith(Global::kSFtp))
        dlg.setDomainLineVisible(false);

    DFMMOUNT::MountPassInfo info;
    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject loginInfo = dlg.getLoginData();
        auto data = loginInfo.toVariantMap();
        using namespace GlobalServerDefines::NetworkMountParamKey;
        if (data.contains(kAnonymous) && data.value(kAnonymous).toBool()) {
            info.anonymous = true;
        } else {
            info.userName = data.value(kUser).toString();
            info.domain = data.value(kDomain).toString();
            info.passwd = data.value(kPasswd).toString();
            info.savePasswd = static_cast<DFMMOUNT::NetworkMountPasswdSaveMode>(data.value(kPasswdSaveMode).toInt());
        }
    } else {
        info.cancelled = true;
    }

    return info;
}

int DeviceManagerPrivate::askForUserChoice(const QString &message, const QStringList &choices)
{
    QString newMsg = message;
    QString title;
    if (message.startsWith("Can’t verify the identity of")
        && message.endsWith("If you want to be absolutely sure it is safe to continue, contact the system administrator.")) {
        QString arg1, arg2;
        QRegularExpression reg("“.*?”");
        auto matcher = reg.match(message);
        if (matcher.hasMatch()) {
            arg1 = matcher.captured(0);
            newMsg = newMsg.replace(arg1, "");

            matcher = reg.match(newMsg);
            arg2 = matcher.captured(0);

            title = QObject::tr("Can’t verify the identity of %1.").arg(arg1);
            newMsg = QObject::tr("This happens when you log in to a computer the first time.") + '\n'
                    + QObject::tr("The identity sent by the remote computer is") + '\n'
                    + arg2 + '\n'
                    + QObject::tr("If you want to be absolutely sure it is safe to continue, contact the system administrator.");
        }
        newMsg = newMsg.replace("\\r\\n", "\n");

        qDebug() << "filtered question message is: " << newMsg;
    }
    DWIDGET_USE_NAMESPACE
    DDialog askForChoice;
    askForChoice.setTitle(title);
    askForChoice.setMessage(newMsg);
    askForChoice.addButtons(choices);
    askForChoice.setMaximumWidth(480);

    return askForChoice.exec();
}
