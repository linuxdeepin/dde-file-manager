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
#ifndef NEWDEVICEMANAGER_H
#define NEWDEVICEMANAGER_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <dfm-mount/base/dmount_global.h>

#include <QObject>
#include <QVariantMap>

#define DevMngIns DFMBASE_NAMESPACE::DeviceManager::instance()

DFMBASE_BEGIN_NAMESPACE

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
    Q_DISABLE_COPY(DeviceManager);

public:
    inline static DeviceManager *instance()
    {
        static DeviceManager ins;
        return &ins;
    }

    // device infos
    QStringList getAllBlockDevID(GlobalServerDefines::DeviceQueryOptions opts = 0);
    QStringList getAllProtocolDevID();
    QVariantMap getBlockDevInfo(const QString &id, bool needReload = false);
    QVariantMap getProtocolDevInfo(const QString &id, bool needReload = false);

    // device operations
    QString mountBlockDev(const QString &id, const QVariantMap &opts = {});
    void mountBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType1 cb = nullptr);
    bool unmountBlockDev(const QString &id, const QVariantMap &opts = {});
    void unmountBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    bool lockBlockDev(const QString &id, const QVariantMap &opts = {});
    void lockBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    QString unlockBlockDev(const QString &id, const QString &passwd, const QVariantMap &opts = {});
    void unlockBlockDevAsync(const QString &id, const QString &passwd, const QVariantMap &opts = {}, CallbackType1 cb = nullptr);
    bool powerOffBlockDev(const QString &id, const QVariantMap &opts = {});
    void powerOffBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    bool ejectBlockDev(const QString &id, const QVariantMap &opts = {});
    void ejectBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    bool renameBlockDev(const QString &id, const QString &newName, const QVariantMap &opts = {});
    void renameBlockDevAsync(const QString &id, const QString &newName, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);
    bool rescanBlockDev(const QString &id, const QVariantMap &opts = {});
    void rescanBlockDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);

    QString mountProtocolDev(const QString &id, const QVariantMap &opts = {});
    void mountProtocolDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType1 cb = nullptr);
    bool unmountProtocolDev(const QString &id, const QVariantMap &opts = {});
    void unmountProtocolDevAsync(const QString &id, const QVariantMap &opts = {}, CallbackType2 cb = nullptr);

    void mountNetworkDeviceAsync(const QString &address, CallbackType1 cb = nullptr, int timeout = 0);

    void doAutoMountAtStart();

    // convinience operations
    void detachAllRemovableBlockDevs();
    QStringList detachBlockDev(const QString &id, CallbackType2 cb = nullptr);
    void detachAllProtoDevs();
    void detachProtoDev(const QString &id);

    void startPollingDeviceUsage();
    void stopPollingDeviceUsage();

    void startMonitor();
    void stopMonitor();
    bool isMonitoring();

    void stopScanDrive(const QString &id);
    void stopAllScanTask();

signals:
    // device watch
    void devSizeChanged(const QString &id, qint64 total, qint64 avai);

    void blockDevPropertyChanged(const QString &id, const QString &property, const QVariant &val);
    void blockDriveAdded();
    void blockDriveRemoved();
    void blockDevAdded(const QString &id);
    void blockDevRemoved(const QString &id);
    void blockDevMounted(const QString &id, const QString &mpt);
    void blockDevUnmounted(const QString &id);
    void blockDevLocked(const QString &id);
    void blockDevUnlocked(const QString &id, const QString &cleartextId);
    void blockDevFsAdded(const QString &id);
    void blockDevFsRemoved(const QString &id);

    // these 3 signals is designed only for dock's opeartion.
    void blockDevUnmountAsyncFailed(const QString &id, DFMMOUNT::DeviceError err);
    void blockDevEjectAsyncFailed(const QString &id, DFMMOUNT::DeviceError err);
    void blockDevPoweroffAysncFailed(const QString &id, DFMMOUNT::DeviceError err);

    void protocolDevAdded(const QString &id);
    void protocolDevRemoved(const QString &id);
    void protocolDevMounted(const QString &id, const QString &mpt);
    void protocolDevUnmounted(const QString &id);

private:
    explicit DeviceManager(QObject *parent = nullptr);
    virtual ~DeviceManager() override;
    void doAutoMount(const QString &id, DFMMOUNT::DeviceType type);

private:
    QScopedPointer<DeviceManagerPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // NEWDEVICEMANAGER_H
