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

#include <mutex>

DSC_BEGIN_NAMESPACE

class DeviceServiceHelper
{
    friend class DeviceService;
private:
    static std::once_flag &autoMountOnceFlag();
    static std::once_flag &connectOnceFlag();
    static dfmbase::Settings *getGsGlobal();
    static void mountAllBlockDevices();
    static bool mountBlockDevice(DFMMOUNT::DFMBlockDevice *blkDev, const QVariantMap &opts);
    static void mountAllProtocolDevices();
    static void unmountAllBlockDevices();
    static void unmountAllProtocolDevices();
    static QList<QUrl> getMountPathForDrive(const QString &driveName);
    static QList<QUrl> getMountPathForAllDrive();
    static QUrl getMountPathForBlock(const DFMMOUNT::DFMBlockDevice *blkDev);
    static bool isMountableBlockDevice(const DFMMOUNT::DFMBlockDevice *blkDev);
    static bool isProtectedBlocDevice(const DFMMOUNT::DFMBlockDevice *blkDev);

private:
    static void showUnmountFailedNotification(DFMMOUNT::MountError err);
    static bool powerOffBlockblockDeivce(DFMMOUNT::DFMBlockDevice *block);
};

DSC_END_NAMESPACE

#endif // DEVICESERVICEHELPER_H
