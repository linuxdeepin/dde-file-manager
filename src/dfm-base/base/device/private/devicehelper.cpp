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
#include "devicehelper.h"
#include "defendercontroller.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/utils/dialogmanager.h"

#include <DDesktopServices>
#include <DDialog>
#include <QDebug>
#include <QStandardPaths>
#include <QProcess>

#include <dfm-mount/dmount.h>
#include <dfm-burn/dburn_global.h>

static constexpr char kBurnAttribute[] { "BurnAttribute" };
static constexpr char kBurnTotalSize[] { "BurnTotalSize" };
static constexpr char kBurnUsedSize[] { "BurnUsedSize" };
static constexpr char kBurnMediaType[] { "BurnMediaType" };
static constexpr char kBurnWriteSpeed[] { "BurnWriteSpeed" };

using namespace dfmbase;
DFM_MOUNT_USE_NS

DevPtr DeviceHelper::createDevice(const QString &devId, dfmmount::DeviceType type)
{
    auto manager = DDeviceManager::instance();
    auto monitor = manager->getRegisteredMonitor(type);
    Q_ASSERT_X(monitor, "DeviceServiceHelper", "dfm-mount return a NULL monitor!");
    return monitor->createDeviceById(devId);
}

BlockDevPtr DeviceHelper::createBlockDevice(const QString &id)
{
    auto devPtr = createDevice(id, DeviceType::kBlockDevice);
    return qobject_cast<BlockDevPtr>(devPtr);
}

ProtocolDevPtr DeviceHelper::createProtocolDevice(const QString &id)
{
    auto devPtr = createDevice(id, DeviceType::kProtocolDevice);
    return qobject_cast<ProtocolDevPtr>(devPtr);
}

QVariantMap DeviceHelper::loadBlockInfo(const QString &id)
{
    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qDebug() << "device is not exist!: " << id;
        return {};
    }
    return loadBlockInfo(dev);
}

QVariantMap DeviceHelper::loadBlockInfo(const BlockDevPtr &dev)
{
    if (!dev) return {};

    auto getNullStrIfNotValid = [&dev](Property p) {
        auto ret = dev->getProperty(p);
        return ret.isValid() ? ret : "";
    };

    using namespace GlobalServerDefines::DeviceProperty;
    using namespace DFMMOUNT;
    QVariantMap datas;
    datas[kId] = dev->path();
    datas[kMountPoint] = dev->mountPoint();
    datas[kFileSystem] = dev->fileSystem();
    datas[kSizeTotal] = dev->sizeTotal();

    datas[kUUID] = getNullStrIfNotValid(Property::kBlockIDUUID);
    datas[kFsVersion] = getNullStrIfNotValid(Property::kBlockIDVersion);
    datas[kDevice] = dev->device();
    datas[kIdLabel] = dev->idLabel();
    datas[kMedia] = getNullStrIfNotValid(Property::kDriveMedia);
    datas[kReadOnly] = getNullStrIfNotValid(Property::kBlockReadOnly);
    datas[kRemovable] = dev->removable();
    datas[kMediaRemovable] = getNullStrIfNotValid(Property::kDriveMediaRemovable);
    datas[kOptical] = dev->optical();
    datas[kOpticalBlank] = dev->opticalBlank();
    datas[kMediaAvailable] = getNullStrIfNotValid(Property::kDriveMediaAvailable);
    datas[kCanPowerOff] = dev->canPowerOff();
    datas[kEjectable] = dev->ejectable();
    datas[kIsEncrypted] = dev->isEncrypted();
    datas[kIsLoopDevice] = dev->isLoopDevice();
    datas[kHasFileSystem] = dev->hasFileSystem();
    datas[kHasPartitionTable] = dev->hasPartitionTable();
    datas[kHasPartition] = dev->hasPartition();
    datas[kHintSystem] = dev->hintSystem();
    datas[kHintIgnore] = dev->hintIgnore();
    datas[kCryptoBackingDevice] = getNullStrIfNotValid(Property::kBlockCryptoBackingDevice);
    datas[kDrive] = dev->drive();
    datas[kMountPoints] = dev->mountPoints();
    datas[kMediaCompatibility] = dev->mediaCompatibility();
    datas[kOpticalDrive] = dev->mediaCompatibility().join(", ").contains("optical");
    datas[kCleartextDevice] = getNullStrIfNotValid(Property::kEncryptedCleartextDevice);
    datas[kConnectionBus] = getNullStrIfNotValid(Property::kDriveConnectionBus);

    if (dev->optical())
        datas[kUDisks2Size] = dev->sizeTotal();

    auto eType = dev->partitionEType();
    datas[kHasExtendedPatition] = eType == PartitionType::kMbrWin95_Extended_LBA
            || eType == PartitionType::kMbrLinux_extended
            || eType == PartitionType::kMbrExtended
            || eType == PartitionType::kMbrDRDOS_sec_extend
            || eType == PartitionType::kMbrMultiuser_DOS_extend;

    if (datas[kOpticalDrive].toBool() && datas[kOptical].toBool())
        readOpticalInfo(datas);

    return datas;
}

QVariantMap DeviceHelper::loadProtocolInfo(const QString &id)
{
    auto dev = DeviceHelper::createProtocolDevice(id);
    if (!dev) {
        qDebug() << "device is not exist!: " << id;
        return {};
    }
    return loadProtocolInfo(dev);
}

QVariantMap DeviceHelper::loadProtocolInfo(const ProtocolDevPtr &dev)
{
    if (!dev) return {};

    using namespace GlobalServerDefines::DeviceProperty;
    using namespace DFMMOUNT;
    QVariantMap datas;
    datas[kId] = dev->path();
    datas[kFileSystem] = dev->fileSystem();
    datas[kSizeTotal] = dev->sizeTotal();
    datas[kSizeUsed] = dev->sizeUsage();
    datas[kSizeFree] = dev->sizeTotal() - dev->sizeUsage();
    datas[kMountPoint] = dev->mountPoint();
    datas[kDisplayName] = dev->displayName();
    datas[kDeviceIcon] = dev->deviceIcons();

    return datas;
}

bool DeviceHelper::isMountableBlockDev(const QString &id, QString &why)
{
    BlockDevPtr dev = createBlockDevice(id);
    return isMountableBlockDev(dev, why);
}

bool DeviceHelper::isMountableBlockDev(const BlockDevPtr &dev, QString &why)
{
    if (!dev) {
        why = "block device is not valid!";
        return false;
    }
    auto &&datas = loadBlockInfo(dev);
    return isMountableBlockDev(datas, why);
}

bool DeviceHelper::isMountableBlockDev(const QVariantMap &infos, QString &why)
{
    using namespace GlobalServerDefines::DeviceProperty;

    if (infos.value(kId).toString().isEmpty()) {
        why = "block id is empty";
        return false;
    }

    if (infos.value(kHintIgnore).toBool()) {
        why = "device is ignored";
        return false;
    }

    if (!infos.value(kMountPoint).toString().isEmpty()) {
        why = "device is already mounted at: " + infos.value(kMountPoint).toString();
        return false;
    }

    if (!infos.value(kHasFileSystem).toBool()) {
        why = "device do not have a filesystem interface";
        return false;
    }

    if (infos.value(kIsEncrypted).toBool()) {
        why = "device is encrypted";
        return false;
    }
    return true;
}

bool DeviceHelper::isEjectableBlockDev(const QString &id, QString &why)
{
    auto dev = createBlockDevice(id);
    return isEjectableBlockDev(dev, why);
}

bool DeviceHelper::isEjectableBlockDev(const BlockDevPtr &dev, QString &why)
{
    if (!dev) {
        why = "device is not valid";
        return false;
    }
    return isEjectableBlockDev(loadBlockInfo(dev), why);
}

bool DeviceHelper::isEjectableBlockDev(const QVariantMap &infos, QString &why)
{
    using namespace GlobalServerDefines::DeviceProperty;

    if (infos.value(kRemovable).toBool())
        return true;
    if (infos.value(kOptical).toBool() && infos.value(kEjectable).toBool())
        return true;

    why = "device is not removable or is not ejectable optical item";
    return false;
}

bool DeviceHelper::askForStopScanning(const QUrl &mpt)
{
    if (!DefenderController::instance().isScanning(mpt))
        return true;

    DDialog *dlg = DialogManagerInstance->showQueryScanningDialog(QObject::tr("Scanning the device, stop it?"));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    if (dlg->exec() == QDialog::Accepted) {
        if (DefenderController::instance().stopScanning(mpt))
            return true;

        qWarning() << "stop scanning device failed: " << mpt;
        DialogManagerInstance->showErrorDialog(QObject::tr("Unmount failed"), QObject::tr("Cannot stop scanning device"));
    }
    return false;
}

void DeviceHelper::openFileManagerToDevice(const QString &blkId, const QString &mpt)
{
    if (!QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
        QString mountPoint = mpt;
        // not auto mount for optical for now.
        //        if (blkId.contains(QRegularExpression("sr[0-9]*$"))) {
        //            mountPoint = QString("burn:///dev/%1/disc_files/").arg(blkId.mid(blkId.lastIndexOf("/") + 1));
        //        }
        QProcess::startDetached(QStringLiteral("dde-file-manager"), { mountPoint });
        qInfo() << "open by dde-file-manager: " << mountPoint;
        return;
    }

    DWIDGET_USE_NAMESPACE
    DDesktopServices::showFolder(QUrl::fromLocalFile(mpt));
}

QString DeviceHelper::castFromDFMMountProperty(dfmmount::Property property)
{
    using namespace GlobalServerDefines;
#define item(a, b) std::make_pair<Property, QString>(Property::a, DeviceProperty::b)
    // these are only mutable properties
    static QMap<Property, QString> mapper {
        item(kBlockSize, kSizeTotal),
        item(kBlockIDUUID, kUUID),
        item(kBlockIDType, kFileSystem),
        item(kBlockIDVersion, kFsVersion),
        item(kBlockIDLabel, kIdLabel),
        item(kDriveMedia, kMedia),
        item(kBlockReadOnly, kReadOnly),
        item(kDriveMediaRemovable, kMediaRemovable),
        item(kDriveOptical, kOptical),
        item(kDriveOpticalBlank, kOpticalBlank),
        item(kDriveMediaAvailable, kMediaAvailable),
        item(kDriveCanPowerOff, kCanPowerOff),
        item(kDriveEjectable, kEjectable),
        item(kBlockHintIgnore, kHintIgnore),
        item(kBlockCryptoBackingDevice, kCryptoBackingDevice),
        item(kFileSystemMountPoint, kMountPoints),
        item(kDriveMediaCompatibility, kMediaCompatibility),
        item(kEncryptedCleartextDevice, kCleartextDevice),
    };
    return mapper.value(property, "");
}

void DeviceHelper::persistentOpticalInfo(const QVariantMap &datas)
{
    using namespace GlobalServerDefines;
    QVariantMap info;
    QString tag { datas.value(DeviceProperty::kDevice).toString().mid(5) };

    info[kBurnTotalSize] = datas.value(DeviceProperty::kSizeTotal);
    info[kBurnUsedSize] = datas.value(DeviceProperty::kSizeUsed);
    info[kBurnMediaType] = datas.value(DeviceProperty::kOpticalMediaType);
    info[kBurnWriteSpeed] = datas.value(DeviceProperty::kOpticalWriteSpeed);

    Application::dataPersistence()->setValue(kBurnAttribute, tag, info);
    Application::dataPersistence()->sync();

    qDebug() << "optical usage persistented: " << datas;
}

void DeviceHelper::readOpticalInfo(QVariantMap &datas)
{
    using namespace GlobalServerDefines;
    QString tag { datas.value(DeviceProperty::kDevice).toString().mid(5) };

    if (Application::dataPersistence()->keys(kBurnAttribute).contains(tag)) {
        const QMap<QString, QVariant> &info = Application::dataPersistence()->value(kBurnAttribute, tag).toMap();
        datas[DeviceProperty::kSizeTotal] = static_cast<qint64>(info.value(kBurnTotalSize).toULongLong());
        datas[DeviceProperty::kSizeUsed] = static_cast<qint64>(info.value(kBurnUsedSize).toULongLong());
        datas[DeviceProperty::kSizeFree] = datas[DeviceProperty::kSizeTotal].toULongLong() - datas[DeviceProperty::kSizeUsed].toULongLong();
        datas[DeviceProperty::kOpticalMediaType] = info.value(kBurnMediaType).toInt();
        datas[DeviceProperty::kOpticalWriteSpeed] = info.value(kBurnWriteSpeed).toStringList();

        qDebug() << "optical usage loaded: " << tag << endl
                 << "sizeTotal: " << datas.value(DeviceProperty::kSizeTotal) << endl
                 << "sizeUsed: " << datas.value(DeviceProperty::kSizeUsed) << endl
                 << "sizeFree: " << datas.value(DeviceProperty::kSizeFree) << endl
                 << "mediaType: " << datas.value(DeviceProperty::kOpticalMediaType) << endl
                 << "speed: " << datas.value(DeviceProperty::kOpticalWriteSpeed);
    }
}
