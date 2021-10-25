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

class DeviceServiceHelper
{
    friend class DeviceService;

    using DevPtr             = QSharedPointer<DFMMOUNT::DFMDevice>;
    using BlockDevPtr        = QSharedPointer<DFMMOUNT::DFMBlockDevice>;
    using ProtocolDevPtr     = QSharedPointer<DFMMOUNT::DFMProtocolDevice>;
    using DevPtrList         = QList<DevPtr>;
    using BlockDevPtrList    = QList<BlockDevPtr>;
    using ProtocolDevPtrList = QList<ProtocolDevPtr>;

private:
    static std::once_flag &autoMountOnceFlag();
    static std::once_flag &connectOnceFlag();
    static dfmbase::Settings *getGsGlobal();

    static void mountAllBlockDevices();
    static bool mountBlockDevice(BlockDevPtr &blkDev, const QVariantMap &opts);
    static void mountAllProtocolDevices();

    static void ejectAllBlockDevices();
    static void ejectAllProtocolDevices();

    static QList<QUrl> getMountPathForDrive(const QString &driveName);
    static QList<QUrl> getMountPathForAllDrive();
    static QUrl getMountPathForBlock(const BlockDevPtr &blkDev);

    static bool isMountableBlockDevice(const BlockDevPtr &blkDev);
    static bool isProtectedBlocDevice(const BlockDevPtr &blkDev);

    static BlockDevPtr        createBlockDevice(const QString &devId);
    static ProtocolDevPtr     createProtocolDevice(const QString &devId);
    static BlockDevPtrList    createAllBlockDevices();
    static ProtocolDevPtrList createAllProtocolDevices();

private:
    static void showEjectFailedNotification(DFMMOUNT::MountError err);
    static bool powerOffBlockblockDeivce(BlockDevPtr &blkDev);
    static DevPtr createDevice(const QString &devId, DFMMOUNT::DeviceType type);
    static DevPtrList createAllDevices(DFMMOUNT::DeviceType type);
};

DSC_END_NAMESPACE

#endif // DEVICESERVICEHELPER_H
