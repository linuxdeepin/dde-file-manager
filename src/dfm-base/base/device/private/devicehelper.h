// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEHELPER_H
#define DEVICEHELPER_H

#include <dfm-base/dfm_base_global.h>

#include <dfm-mount/base/dmount_global.h>
#include <dfm-mount/dprotocoldevice.h>

#include <QSharedPointer>

DFM_MOUNT_BEGIN_NS
class DDevice;
class DBlockDevice;
class DProtocolDevice;
DFM_MOUNT_END_NS

#define DEVICE_SIZE_DISPLAY_BY_DISK 0
#define DEVICE_SIZE_DISPLAY_BY_FS 1

namespace dfmbase {

using DevAutoPtr = QSharedPointer<DFMMOUNT::DDevice>;
using BlockDevAutoPtr = QSharedPointer<DFMMOUNT::DBlockDevice>;
using ProtocolDevAutoPtr = QSharedPointer<DFMMOUNT::DProtocolDevice>;

/*!
 * \brief The DeviceHelper class
 * this class is designed to provide some data obtain functions, some prejudge functions
 * and to reduce the line count of DeviceManager
 */
class DeviceHelper
{
public:
    static DevAutoPtr createDevice(const QString &devId, DFMMOUNT::DeviceType type);
    static BlockDevAutoPtr createBlockDevice(const QString &id);
    static ProtocolDevAutoPtr createProtocolDevice(const QString &id);

    static QVariantMap loadBlockInfo(const QString &id);
    static QVariantMap loadBlockInfo(const BlockDevAutoPtr &dev);
    static QVariantMap loadProtocolInfo(const QString &id);
    static QVariantMap loadProtocolInfo(const ProtocolDevAutoPtr &dev);

    static bool isMountableBlockDev(const QString &id, QString &why);
    static bool isMountableBlockDev(const BlockDevAutoPtr &dev, QString &why);
    static bool isMountableBlockDev(const QVariantMap &infos, QString &why);

    static bool isEjectableBlockDev(const QString &id, QString &why);
    static bool isEjectableBlockDev(const BlockDevAutoPtr &dev, QString &why);
    static bool isEjectableBlockDev(const QVariantMap &infos, QString &why);

    static bool askForStopScanning(const QUrl &mpt);
    static void openFileManagerToDevice(const QString &blkId, const QString &mpt);

    static QString castFromDFMMountProperty(DFMMOUNT::Property property);

    static void persistentOpticalInfo(const QVariantMap &datas);
    static void readOpticalInfo(QVariantMap &datas);

private:
    static bool checkNetworkConnection(const QString &id);
    static QVariantMap makeFakeProtocolInfo(const QString &id);
};

}

#endif   // DEVICEHELPER_H
