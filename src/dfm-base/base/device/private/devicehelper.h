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
#ifndef DEVICEHELPER_H
#define DEVICEHELPER_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-mount/base/dmount_global.h>
#include <dfm-mount/dprotocoldevice.h>

#include <QSharedPointer>

DFM_MOUNT_BEGIN_NS
class DDevice;
class DBlockDevice;
class DProtocolDevice;
DFM_MOUNT_END_NS

DFMBASE_BEGIN_NAMESPACE

using DevPtr = QSharedPointer<DFMMOUNT::DDevice>;
using BlockDevPtr = QSharedPointer<DFMMOUNT::DBlockDevice>;
using ProtocolDevPtr = QSharedPointer<DFMMOUNT::DProtocolDevice>;

/*!
 * \brief The DeviceHelper class
 * this class is designed to provide some data obtain functions, some prejudge functions
 * and to reduce the line count of DeviceManager
 */
class DeviceHelper
{
public:
    static DevPtr createDevice(const QString &devId, DFMMOUNT::DeviceType type);
    static BlockDevPtr createBlockDevice(const QString &id);
    static ProtocolDevPtr createProtocolDevice(const QString &id);

    static QVariantMap loadBlockInfo(const QString &id);
    static QVariantMap loadBlockInfo(const BlockDevPtr &dev);
    static QVariantMap loadProtocolInfo(const QString &id);
    static QVariantMap loadProtocolInfo(const ProtocolDevPtr &dev);

    static bool isMountableBlockDev(const QString &id, QString &why);
    static bool isMountableBlockDev(const BlockDevPtr &dev, QString &why);
    static bool isMountableBlockDev(const QVariantMap &infos, QString &why);

    static bool isEjectableBlockDev(const QString &id, QString &why);
    static bool isEjectableBlockDev(const BlockDevPtr &dev, QString &why);
    static bool isEjectableBlockDev(const QVariantMap &infos, QString &why);

    static bool askForStopScanning(const QUrl &mpt);
    static void openFileManagerToDevice(const QString &blkId, const QString &mpt);

    static QString castFromDFMMountProperty(DFMMOUNT::Property property);

    static void persistentOpticalInfo(const QVariantMap &datas);
    static void readOpticalInfo(QVariantMap &datas);
};

DFMBASE_END_NAMESPACE

#endif   // DEVICEHELPER_H
