// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

namespace dfmbase {

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

}

#endif   // DEVICEHELPER_H
