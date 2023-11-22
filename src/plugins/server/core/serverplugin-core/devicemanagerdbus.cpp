// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicemanagerdbus.h"
#include "serverplugin_core_global.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-io/denumerator.h>
#include <dfm-io/dfileinfo.h>

#include <QDBusInterface>
#include <QDebug>

SERVERPCORE_USE_NAMESPACE
DFM_LOG_REISGER_CATEGORY(SERVERPCORE_NAMESPACE)
DFMBASE_USE_NAMESPACE

using namespace GlobalServerDefines;

DeviceManagerDBus::DeviceManagerDBus(QObject *parent)
    : QObject(parent)
{
    initialize();
    initConnection();
    DevMngIns->doAutoMountAtStart();
}

bool DeviceManagerDBus::IsMonotorWorking()
{
    return DevMngIns->isMonitoring();
}

void DeviceManagerDBus::DetachBlockDevice(QString id)
{
    DevMngIns->detachBlockDev(id);
}

void DeviceManagerDBus::DetachProtocolDevice(QString id)
{
    DevMngIns->detachProtoDev(id);
}

void DeviceManagerDBus::initialize()
{
    DevMngIns->startMonitor();
    DevMngIns->startPollingDeviceUsage();
}

/*!
 * \brief pub signals with device monitor
 */
void DeviceManagerDBus::initConnection()
{
    connect(DevMngIns, &DeviceManager::blockDevUnmountAsyncFailed, this, [this](const QString &deviceId, DFMMOUNT::DeviceError err) {
        Q_UNUSED(deviceId)
        Q_UNUSED(err)
        emit NotifyDeviceBusy(DeviceBusyAction::kUnmount);
    });
    connect(DevMngIns, &DeviceManager::blockDevEjectAsyncFailed, this, [this](const QString &deviceId, DFMMOUNT::DeviceError err) {
        Q_UNUSED(deviceId)
        Q_UNUSED(err)
        emit NotifyDeviceBusy(DeviceBusyAction::kEject);
    });
    connect(DevMngIns, &DeviceManager::blockDevPoweroffAysncFailed, this, [this](const QString &deviceId, DFMMOUNT::DeviceError err) {
        Q_UNUSED(deviceId)
        Q_UNUSED(err)
        emit NotifyDeviceBusy(DeviceBusyAction::kRemove);
    });

    connect(DevMngIns, &DeviceManager::devSizeChanged, this, &DeviceManagerDBus::SizeUsedChanged);
    connect(DevMngIns, &DeviceManager::blockDriveAdded, this, &DeviceManagerDBus::BlockDriveAdded);
    connect(DevMngIns, &DeviceManager::blockDriveRemoved, this, &DeviceManagerDBus::BlockDriveRemoved);
    connect(DevMngIns, &DeviceManager::blockDevAdded, this, &DeviceManagerDBus::BlockDeviceAdded);
    connect(DevMngIns, &DeviceManager::blockDevFsAdded, this, &DeviceManagerDBus::BlockDeviceFilesystemAdded);
    connect(DevMngIns, &DeviceManager::blockDevFsRemoved, this, &DeviceManagerDBus::BlockDeviceFilesystemRemoved);
    connect(DevMngIns, &DeviceManager::blockDevUnlocked, this, &DeviceManagerDBus::BlockDeviceUnlocked);
    connect(DevMngIns, &DeviceManager::blockDevLocked, this, &DeviceManagerDBus::BlockDeviceLocked);
    connect(DevMngIns, &DeviceManager::blockDevPropertyChanged, this, [this](const QString &id, const QString &property, const QVariant &val) {
        if (!val.isNull() && val.isValid())
            emit this->BlockDevicePropertyChanged(id, property, QDBusVariant(val));
    });

    connect(DevMngIns, &DeviceManager::protocolDevMounted, this, &DeviceManagerDBus::ProtocolDeviceMounted);
    connect(DevMngIns, &DeviceManager::protocolDevUnmounted, this, &DeviceManagerDBus::ProtocolDeviceUnmounted);
    connect(DevMngIns, &DeviceManager::protocolDevAdded, this, &DeviceManagerDBus::ProtocolDeviceAdded);
    connect(DevMngIns, &DeviceManager::protocolDevRemoved, this, &DeviceManagerDBus::ProtocolDeviceRemoved);

    connect(DevMngIns, &DeviceManager::blockDevMounted, this, [this](const QString &id, const QString &mpt) {
        emit BlockDeviceMounted(id, mpt);
        requestRefreshDesktopAsNeeded(mpt, "onMount");
    });
    connect(DevMngIns, &DeviceManager::blockDevUnmounted, this, [this](const QString &id, const QString &oldMpt) {
        emit BlockDeviceUnmounted(id, oldMpt);
        requestRefreshDesktopAsNeeded(oldMpt, "onUnmount");
    });
    connect(DevMngIns, &DeviceManager::blockDevRemoved, this, [this](const QString &id, const QString &oldMpt) {
        emit BlockDeviceRemoved(id, oldMpt);
        requestRefreshDesktopAsNeeded(oldMpt, "onRemove");
    });
}

void DeviceManagerDBus::requestRefreshDesktopAsNeeded(const QString &path, const QString &operation)
{
    QString desktopPath = StandardPaths::location(StandardPaths::kDesktopPath);
    if (desktopPath.isEmpty())
        return;

    fmDebug() << "looking for link files from" << desktopPath;
    dfmio::DEnumerator enu(QUrl::fromLocalFile(desktopPath));
    auto files = enu.fileInfoList();
    bool hasFileLinkToTarget = std::any_of(files.cbegin(), files.cend(), [path](QSharedPointer<dfmio::DFileInfo> file) {
        bool isSym = file->attribute(dfmio::DFileInfo::AttributeID::kStandardIsSymlink).toBool();
        if (!isSym) return false;
        auto target = file->attribute(dfmio::DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
        return target.startsWith(path);
    });
    if (hasFileLinkToTarget) {
        QDBusInterface ifs("com.deepin.dde.desktop",
                           "/com/deepin/dde/desktop",
                           "com.deepin.dde.desktop");
        ifs.asyncCall("Refresh");
        fmInfo() << "refresh desktop async finished..." << operation << path;
    }
}

/*!
 * \brief this is convience interface for `disk-mount` plugin
 */
void DeviceManagerDBus::DetachAllMountedDevices()
{
    DevMngIns->detachAllRemovableBlockDevs();
    DevMngIns->detachAllProtoDevs();
}

/*!
 * \brief user input a opts, then return devices list
 * \param opts: refrecne to DeviceService::blockDevicesIdList
 * \return devices id list
 */
QStringList DeviceManagerDBus::GetBlockDevicesIdList(int opts)
{
    return DevMngIns->getAllBlockDevID(static_cast<DeviceQueryOptions>(opts));
}

QVariantMap DeviceManagerDBus::QueryBlockDeviceInfo(QString id, bool reload)
{
    return DevMngIns->getBlockDevInfo(id, reload);
}

QStringList DeviceManagerDBus::GetProtocolDevicesIdList()
{
    return DevMngIns->getAllProtocolDevID();
}

QVariantMap DeviceManagerDBus::QueryProtocolDeviceInfo(QString id, bool reload)
{
    return DevMngIns->getProtocolDevInfo(id, reload);
}
