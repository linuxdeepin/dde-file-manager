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
#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "dfm_common_service_global.h"

#include <dfm-framework/service/pluginservicecontext.h>

DSC_BEGIN_NAMESPACE
class DeviceMonitorHandler;
class DeviceService final : public dpf::PluginService, dpf::AutoServiceRegister<DeviceService>
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.DeviceService";
    }

public:   // operations
    void startAutoMount();
    bool startMonitor();
    bool stopMonitor();
    bool stopDefenderScanDrive(const QString &deviceId);
    bool stopDefenderScanAllDrives();

    void detachBlockDevice(const QString &deviceId);
    void detachProtocolDevice(const QString &deviceId);
    void detachAllMountedBlockDevices();
    void detachAllMountedProtocolDevices();
    void mountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {});
    QString mountBlockDevice(const QString &deviceId, const QVariantMap &opts = {});
    void unmountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {});
    bool unmountBlockDevice(const QString &deviceId, const QVariantMap &opts = {});
    void ejectBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {});
    bool ejectBlockDevice(const QString &deviceId, const QVariantMap &opts = {});
    void poweroffBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {});
    bool poweroffBlockDevice(const QString &deviceId, const QVariantMap &opts = {});
    bool renameBlockDevice(const QString &deviceId, const QString &newName, const QVariantMap &opts = {});
    void renameBlockDeviceAsync(const QString &deviceId, const QString &newName, const QVariantMap &opts = {});

    QString unlockBlockDevice(const QString &passwd, const QString &deviceId, const QVariantMap &opts = {});
    void unlockBlockDeviceAsync(const QString &passwd, const QString &deviceId, const QVariantMap &opts = {});
    bool preLockBlock(const QString &deviceId);
    bool lockBlockDevice(const QString &deviceId, const QVariantMap &opts = {});
    void lockBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts = {});

    QString mountProtocolDevice(const QString &deviceId, const QVariantMap &opts = {});
    void mountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts = {});
    bool unmountProtocolDevice(const QString &deviceId, const QVariantMap &opts = {});
    void unmountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts = {});

    void mountNetworkDevice(const QString &address, bool anonymous, const QVariantMap &opts);

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
    explicit DeviceService(QObject *parent = nullptr);
    virtual ~DeviceService() override;

private:
    QScopedPointer<DeviceMonitorHandler> monitorHandler;
};

DSC_END_NAMESPACE

#endif   // DEVICESERVICE_H
