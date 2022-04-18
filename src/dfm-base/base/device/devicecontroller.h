/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-mount/base/dfmmount_global.h>

#include <QObject>
#include <QVariantMap>

DFMBASE_BEGIN_NAMESPACE
class DeviceMonitorHandler;
class DeviceController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceController)

public:
    static DeviceController *instance();

public:   // operations
    void startAutoMount();
    bool startMonitor();
    bool stopMonitor();
    bool stopDefenderScanDrive(const QString &deviceId);
    bool stopDefenderScanAllDrives();
    void disableStorageInfoPoll();

    bool detachBlockDevice(const QString &deviceId);
    bool detachProtocolDevice(const QString &deviceId);
    bool detachAllMountedBlockDevices();
    bool detachAllMountedProtocolDevices();

    void mountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallbackWithMessage callback = nullptr);
    QString mountBlockDevice(const QString &deviceId, const QVariantMap &opts = {});

    void unmountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallback callback = nullptr);
    bool unmountBlockDevice(const QString &deviceId, const QVariantMap &opts = {});

    void ejectBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallback callback = nullptr);
    bool ejectBlockDevice(const QString &deviceId, const QVariantMap &opts = {});

    void poweroffBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallback callback = nullptr);
    bool poweroffBlockDevice(const QString &deviceId, const QVariantMap &opts = {});

    void renameBlockDeviceAsync(const QString &deviceId, const QString &newName, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallback callback = nullptr);
    bool renameBlockDevice(const QString &deviceId, const QString &newName, const QVariantMap &opts = {});

    QString unlockBlockDevice(const QString &passwd, const QString &deviceId, const QVariantMap &opts = {});
    void unlockBlockDeviceAsync(const QString &passwd, const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallbackWithMessage callback = nullptr);

    bool lockBlockDevice(const QString &deviceId, const QVariantMap &opts = {});
    void lockBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallback callback = nullptr);

    void mountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallbackWithMessage callback = nullptr);
    QString mountProtocolDevice(const QString &deviceId, const QVariantMap &opts = {});

    void unmountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts = {}, DFMMOUNT::DeviceOperateCallback callback = nullptr);
    bool unmountProtocolDevice(const QString &deviceId, const QVariantMap &opts = {});

    // timeout == 0 means never timed out
    void mountNetworkDevice(const QString &address, DFMMOUNT::DeviceOperateCallbackWithMessage callback, int timeout = 0);
    bool rescanDevice(const QString &deviceId, const QVariantMap &opts = {});

public:   // status
    bool isBlockDeviceMonitorWorking() const;
    bool isProtolDeviceMonitorWorking() const;
    bool isInLiveSystem() const;
    bool isAutoMountSetting() const;
    bool isAutoMountAndOpenSetting() const;
    bool isDefenderScanningDrive(const QString &deviceId = "") const;
    QStringList blockDevicesIdList(const QVariantMap &opts) const;
    QVariantMap blockDeviceInfo(const QString &deviceId, bool detail = false) const;
    QStringList protocolDevicesIdList() const;
    QVariantMap protocolDeviceInfo(const QString &deviceId, bool detail = false) const;
    QString getErrorMessage(DFMMOUNT::DeviceError err);

public:   // special
    void ghostBlockDevMounted(const QString &deviceId, const QString &mountPoint);

signals:
    void deviceSizeUsedChanged(const QString &deviceId, qint64 total, qint64 free);
    void blockDevicePropertyChanged(const QString &deviceId, const QString &property, const QVariant &val);
    void blockDriveAdded();
    void blockDriveRemoved();
    void blockDevAdded(const QString &deviceId);
    void blockDevRemoved(const QString &deviceId);
    void blockDevFilesystemAdded(const QString &deviceId);
    void blockDevFilesystemRemoved(const QString &deviceId);
    void blockDevMounted(const QString &deviceId, const QString &mountPoint);
    void blockDevUnmounted(const QString &deviceId);
    void blockDevLocked(const QString &deviceId);
    void blockDevUnlocked(const QString &deviceId, const QString &cleartextBlkId);

    void blockDevAsyncMounted(const QString &deviceId, bool success);
    void blockDevAsyncUnmounted(const QString &deviceId, bool success);
    void blockDevAsyncEjected(const QString &deviceId, bool success);
    void blockDevAsyncPoweroffed(const QString &deviceId, bool success);
    void blockDevAsyncUnlocked(const QString &deviceId, const QString &cleartextBlkId, bool success);
    void blockDevAsyncLocked(const QString &deviceId, bool success);
    void blockDevAsyncRenamed(const QString &deviceId, bool success);

    void protocolDevAdded(const QString &deviceId);
    void protocolDevRemoved(const QString &deviceId);
    void protocolDevMounted(const QString &deviceId, const QString &mountpoint);
    void protocolDevUnmounted(const QString &deviceId);

    void protocolDevAsyncMounted(const QString &deviceId, bool success);
    void protocolDevAsyncUnmounted(const QString &deviceId, bool success);

private:
    explicit DeviceController(QObject *parent = nullptr);
    virtual ~DeviceController() override;

    bool askForStopScanning(const QString &deviceId);

private:
    QScopedPointer<DeviceMonitorHandler> monitorHandler;
};

DFMBASE_END_NAMESPACE

#endif   // DEVICECONTROLLER_H
