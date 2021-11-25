/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DEVICESERVICEHELPER_H
#define DEVICESERVICEHELPER_H

#include "dfm_common_service_global.h"

#include "dfm-base/application/settings.h"
#include "dfm-base/utils/fileutils.h"

#include <dfm-mount/dfmdevicemanager.h>
#include <dfm-mount/dfmblockdevice.h>
#include <dfm-mount/dfmprotocoldevice.h>

#include <mutex>

DSC_BEGIN_NAMESPACE

struct DeviceData
{
    QString id;
    QString mountpoint;
    QString filesystem;
    qint64 sizeTotal;
    qint64 sizeFree;
    qint64 sizeUsed;
};

struct BlockDeviceData
{
    QString uuid;
    DeviceData common;
    QString fsVersion;
    QStringList mountpoints;
    QString device;
    QString drive;
    QString idLabel;
    QString media;
    QStringList mediaCompatibility;
    bool readOnly;
    bool removable;
    bool mediaRemovable;
    bool optical;
    bool opticalDrive;
    bool opticalBlank;
    bool mediaAvailable;
    bool canPowerOff;
    bool ejectable;
    bool isEncrypted;
    bool isLoopDevice;
    bool hasFileSystem;
    bool hasPartitionTable;
    bool hintSystem;
    bool hintIgnore;
    QString cryptoBackingDevice;
};

struct ProtocolDeviceData
{
    DeviceData common;
};

class DeviceServiceHelper
{
    friend class DeviceService;
    friend class DeviceMonitorHandler;

    using DevPtr = QSharedPointer<DFMMOUNT::DFMDevice>;
    using BlockDevPtr = QSharedPointer<DFMMOUNT::DFMBlockDevice>;
    using ProtocolDevPtr = QSharedPointer<DFMMOUNT::DFMProtocolDevice>;
    using DevPtrList = QList<DevPtr>;
    using BlockDevPtrList = QList<BlockDevPtr>;
    using ProtocolDevPtrList = QList<ProtocolDevPtr>;

private:
    static std::once_flag &autoMountOnceFlag();
    static dfmbase::Settings *getGsGlobal();
    static void openFileManagerToDevice(const BlockDevPtr &blkDev);

    static QList<QUrl> makeMountpointsForDrive(const QString &driveName);
    static QList<QUrl> makeMountpointsForAllDrive();
    static QUrl makeMountpointForBlock(const BlockDevPtr &blkDev);
    static QStringList makeAllDevicesIdForDrive(const QString &driveName);

    static bool isMountableBlockDevice(const BlockDevPtr &blkDev, QString *errMsg = nullptr);
    static bool isMountableBlockDevice(const BlockDeviceData &data, QString *errMsg = nullptr);
    static bool isUnmountableBlockDevice(const BlockDevPtr &blkDev, QString *errMsg = nullptr);
    static bool isUnmountableBlockDevice(const BlockDeviceData &data, QString *errMsg = nullptr);
    static bool isEjectableBlockDevice(const BlockDevPtr &blkDev);
    static bool isEjectableBlockDevice(const BlockDeviceData &data);
    static bool isCanPoweroffBlockDevice(const BlockDevPtr &blkDev);
    static bool isCanPoweroffBlockDevice(const BlockDeviceData &data);
    static bool isProtectedBlocDevice(const BlockDeviceData &data);
    static bool isIgnorableBlockDevice(const BlockDeviceData &data, QString *errMsg = nullptr);

    static BlockDevPtr createBlockDevice(const QString &devId);
    static ProtocolDevPtr createProtocolDevice(const QString &devId);
    static BlockDevPtrList createAllBlockDevices();
    static ProtocolDevPtrList createAllProtocolDevices();

    static void makeBlockDeviceData(const BlockDevPtr &ptr, BlockDeviceData *data);
    static void makeBlockDeviceMap(const BlockDeviceData &data, QVariantMap *map, bool detail = false);
    static void updateBlockDeviceSizeUsed(BlockDeviceData *data, qint64 total, qint64 free);

    // TODO(zhangs): makeProtolDeviceData
    // TODO(zhangs): makeProtocolDeviceMap

private:
    static DevPtr createDevice(const QString &devId, DFMMOUNT::DeviceType type);
    static DevPtrList createAllDevices(DFMMOUNT::DeviceType type);
};

DSC_END_NAMESPACE

#endif   // DEVICESERVICEHELPER_H
