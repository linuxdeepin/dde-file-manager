// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NEWDEVICEMANAGER_H
#define NEWDEVICEMANAGER_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-mount/base/dmount_global.h>

#include <QObject>
#include <QVariantMap>

#define DevMngIns DFMBASE_NAMESPACE::DeviceManager::instance()

namespace dfmbase {

namespace OperateParamField {
inline constexpr char kUnmountWithoutLock[] { "unmount_without_lock" };
}

typedef DFMMOUNT::DeviceOperateCallbackWithMessage CallbackType1;
typedef DFMMOUNT::DeviceOperateCallback CallbackType2;

class DeviceManagerPrivate;

/*!
 * \brief The DeviceManager class
 * this class provide basic operations of watch device, get device info,
 * and operate devices.
 * and provide the functions to DBus layer.
 */
class DeviceManager : public QObject
{
    friend class DeviceWatcher;
    Q_OBJECT
    Q_DISABLE_COPY(DeviceManager)

public:
    static DeviceManager *instance();

    // device infos
    QStringList getAllBlockDevID(GlobalServerDefines::DeviceQueryOptions opts = {});
    QStringList getAllProtocolDevID();
    QVariantMap getBlockDevInfo(const QString &id, bool needReload = false);
    QVariantMap getProtocolDevInfo(const QString &id, bool needReload = false);

    // device operations
#ifdef ENABLE_MOUNT_SYNC_FUNCTIONS
    QString mountBlockDev(const QString &id, const QVariantMap &opts = {});
    bool lockBlockDev(const QString &id, const QVariantMap &opts = {});
    QString unlockBlockDev(const QString &id, const QString &passwd, const QVariantMap &opts = {});
    bool powerOffBlockDev(const QString &id, const QVariantMap &opts = {});
    bool renameBlockDev(const QString &id, const QString &newName, const QVariantMap &opts = {});
    bool ejectBlockDev(const QString &id, const QVariantMap &opts = {});
#endif

    void mountBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType1 cb = nullptr, int timeout = 0);
    bool unmountBlockDev(const QString &id, const QVariantMap &opts = {});
    void unmountBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    void lockBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    void unlockBlockDevAsync(const QString &id, const QString &passwd, const QVariantMap &opts = {}, CallbackType1 cb = nullptr);
    void powerOffBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    void ejectBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    void renameBlockDevAsync(const QString &id, const QString &newName, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    bool rescanBlockDev(const QString &id, const QVariantMap &opts = {});
    void rescanBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);

    QString mountProtocolDev(const QString &id, const QVariantMap &opts = {});
    void mountProtocolDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType1 cb = nullptr);
    bool unmountProtocolDev(const QString &id, const QVariantMap &opts = {});
    void unmountProtocolDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);

    void mountNetworkDeviceAsync(const QString &address, CallbackType1 cb = nullptr, int timeout = 3);

    void doAutoMountAtStart();

    // convinience operations
    void detachAllRemovableBlockDevs();
    QStringList detachBlockDev(const QString &id, CallbackType2 cb = nullptr);
    void detachAllProtoDevs();
    void detachProtoDev(const QString &id);

    void startPollingDeviceUsage();
    void stopPollingDeviceUsage();
    void enableBlockAutoMount();

    void startMonitor();
    void stopMonitor();
    bool isMonitoring();
    void startOpticalDiscScan();   // call once

    void stopScanDrive(const QString &id);
    void stopAllScanTask();

    void retryMount(const QString &id, DFMMOUNT::DeviceType type, int timeout);

Q_SIGNALS:
    // device watch
    void devSizeChanged(const QString &id, qint64 total, qint64 avai);

    void blockDevPropertyChanged(const QString &id, const QString &property, const QVariant &val);
    void blockDriveAdded();
    void blockDriveRemoved();
    void blockDevAdded(const QString &id);
    void blockDevRemoved(const QString &id, const QString &oldMpt);
    void blockDevMounted(const QString &id, const QString &mpt);
    void blockDevUnmounted(const QString &id, const QString &oldMpt);
    void blockDevLocked(const QString &id);
    void blockDevUnlocked(const QString &id, const QString &cleartextId);
    void blockDevFsAdded(const QString &id);
    void blockDevFsRemoved(const QString &id);
    void blockDevMountResult(const QString &id, bool result);

    // these 3 signals is designed only for dock's opeartion.
    void blockDevUnmountAsyncFailed(const QString &id, DFMMOUNT::DeviceError err);
    void blockDevEjectAsyncFailed(const QString &id, DFMMOUNT::DeviceError err);
    void blockDevPoweroffAysncFailed(const QString &id, DFMMOUNT::DeviceError err);

    void protocolDevAdded(const QString &id);
    void protocolDevRemoved(const QString &id, const QString &oldMpt);
    void protocolDevMounted(const QString &id, const QString &mpt);
    void protocolDevUnmounted(const QString &id, const QString &oldMpt);

    void opticalDiscWorkStateChanged(const QString &id, const QString &dev, bool working);

    void mountNetworkDeviceResult(bool ret, DFMMOUNT::DeviceError err, const QString &msg);

    void blockDevMountedManually(const QString &id, const QString &mpt);

private:
    explicit DeviceManager(QObject *parent = nullptr);
    virtual ~DeviceManager() override;
    void doAutoMount(const QString &id, DFMMOUNT::DeviceType type, int timeout = 0);

private:
    QScopedPointer<DeviceManagerPrivate> d;
};

}

#endif   // NEWDEVICEMANAGER_H
