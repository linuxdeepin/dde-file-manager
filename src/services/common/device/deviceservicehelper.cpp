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
#include "deviceservicehelper.h"

#include <QDebug>

Q_GLOBAL_STATIC_WITH_ARGS(dfmbase::Settings, gsGlobal, ("deepin/dde-file-manager", dfmbase::Settings::GenericConfig))

DSC_BEGIN_NAMESPACE

dfmbase::Settings *DeviceServiceHelper::getGsGlobal()
{
    return gsGlobal;
}

std::once_flag &DeviceServiceHelper::onceFlag()
{
    static std::once_flag flag;
    return flag;
}

void DeviceServiceHelper::mountAllBlockDevices()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    // TODO(zhangs): wait dfm-mount change monitor
    QList<DFMMOUNT::DFMDevice *> blkDevcies = manager->devices(DFMMOUNT::DeviceType::BlockDevice);
    for (auto *dev : blkDevcies) {
        // TODO(zhangs): wait dfm-mount add isEncrypted
        bool hintIgnore = dev->getProperty(DFMMOUNT::Property::BlockHintIgnore).toBool();
        QString &&cryptoDev = dev->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString();

        if (cryptoDev.length() > 1)
            continue;
        if (hintIgnore)
            continue;

        // TODO(zhangs): wait dfm-mount change mountPoint interface
        QStringList &&mountPoints = dev->getProperty(DFMMOUNT::Property::FileSystemMountPoint).toStringList();
        bool hasFS = !dev->fileSystem().isEmpty();
        if (hasFS && mountPoints.isEmpty()) {
            QUrl &&mp = dev->mount({{"auth.no_user_interaction", true}});
            qInfo() << "Auto mount block device to: " << mp;
        }
    }
}

void DeviceServiceHelper::mountAllProtocolDevices()
{
    // TODO(zhangs): auto mount protocol devices
}

DSC_END_NAMESPACE
