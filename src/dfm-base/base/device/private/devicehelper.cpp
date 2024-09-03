// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicehelper.h"
#include "defendercontroller.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dialogs/mountpasswddialog/mountaskpassworddialog.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/networkutils.h>

#include <DDesktopServices>
#include <dtkwidget_global.h>
#include <dtkgui_global.h>
#include <DDialog>
#include <QDebug>
#include <QStandardPaths>
#include <QProcess>
#include <QTextStream>

#include <dfm-mount/dmount.h>
#include <dfm-burn/dburn_global.h>
#include <sys/statvfs.h>

static constexpr char kBurnAttribute[] { "BurnAttribute" };
static constexpr char kBurnTotalSize[] { "BurnTotalSize" };
static constexpr char kBurnUsedSize[] { "BurnUsedSize" };
static constexpr char kBurnMediaType[] { "BurnMediaType" };
static constexpr char kBurnWriteSpeed[] { "BurnWriteSpeed" };

using namespace dfmbase;
DFM_MOUNT_USE_NS
// see: https://github.com/linuxdeepin/dtk/issues/134
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

DevAutoPtr DeviceHelper::createDevice(const QString &devId, dfmmount::DeviceType type)
{
    if (type == DeviceType::kAllDevice)
        return nullptr;

    auto manager = DDeviceManager::instance();
    auto monitor = manager->getRegisteredMonitor(type);
    Q_ASSERT_X(monitor, "DeviceServiceHelper", "dfm-mount return a NULL monitor!");
    return monitor->createDeviceById(devId);
}

BlockDevAutoPtr DeviceHelper::createBlockDevice(const QString &id)
{
    auto devPtr = createDevice(id, DeviceType::kBlockDevice);
    return qobject_cast<BlockDevAutoPtr>(devPtr);
}

ProtocolDevAutoPtr DeviceHelper::createProtocolDevice(const QString &id)
{
    auto devPtr = createDevice(id, DeviceType::kProtocolDevice);
    return qobject_cast<ProtocolDevAutoPtr>(devPtr);
}

QVariantMap DeviceHelper::loadBlockInfo(const QString &id)
{
    auto dev = DeviceHelper::createBlockDevice(id);
    if (!dev) {
        qCWarning(logDFMBase) << "device is not exist!: " << id;
        return QVariantMap();
    }
    return loadBlockInfo(dev);
}

QVariantMap DeviceHelper::loadBlockInfo(const BlockDevAutoPtr &dev)
{
    if (!dev) return QVariantMap();

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
    datas[kDriveModel] = getNullStrIfNotValid(Property::kDriveModel);
    datas[kPreferredDevice] = getNullStrIfNotValid(Property::kBlockPreferredDevice);

    datas[kUDisks2Size] = dev->sizeTotal();
    auto mpt = dev->mountPoint();
    if (!mpt.isEmpty() && !dev->optical()) {
        auto type = DConfigManager::instance()->value("org.deepin.dde.file-manager.mount",
                                                      "deviceCapacityDisplay",
                                                      DEVICE_SIZE_DISPLAY_BY_DISK)
                            .toInt();
        if (type == DEVICE_SIZE_DISPLAY_BY_FS) {
            struct statvfs fsInfo;
            int ok = statvfs(mpt.toStdString().c_str(), &fsInfo);
            if (ok == 0) {
                const quint64 blksize = quint64(fsInfo.f_frsize);
                auto total = fsInfo.f_blocks * blksize;
                datas[kSizeTotal] = total;
            }
        }
    }

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
        qCWarning(logDFMBase) << "device is not exist!: " << id;
        return {};
    }
    return loadProtocolInfo(dev);
}

QVariantMap DeviceHelper::loadProtocolInfo(const ProtocolDevAutoPtr &dev)
{
    if (!dev) return {};

    using namespace GlobalServerDefines::DeviceProperty;
    using namespace DFMMOUNT;
    QVariantMap datas;
    if (!checkNetworkConnection(dev->path()))
        return makeFakeProtocolInfo(dev->path());

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
    BlockDevAutoPtr dev = createBlockDevice(id);
    return isMountableBlockDev(dev, why);
}

bool DeviceHelper::isMountableBlockDev(const BlockDevAutoPtr &dev, QString &why)
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

bool DeviceHelper::isEjectableBlockDev(const BlockDevAutoPtr &dev, QString &why)
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

        qCWarning(logDFMBase) << "stop scanning device failed: " << mpt;
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
        qCInfo(logDFMBase) << "open by dde-file-manager: " << mountPoint;
        return;
    }

    DDesktopServices::showFolder(QUrl::fromLocalFile(mpt));
}

QString DeviceHelper::castFromDFMMountProperty(dfmmount::Property property)
{
    using namespace GlobalServerDefines;
    // these are only mutable properties
    static QMap<Property, QString> mapper { { Property::kBlockSize, DeviceProperty::kSizeTotal },
                                            { Property::kBlockIDUUID, DeviceProperty::kUUID },
                                            { Property::kBlockIDType, DeviceProperty::kFileSystem },
                                            { Property::kBlockIDVersion, DeviceProperty::kFsVersion },
                                            { Property::kBlockIDLabel, DeviceProperty::kIdLabel },
                                            { Property::kDriveMedia, DeviceProperty::kMedia },
                                            { Property::kBlockReadOnly, DeviceProperty::kReadOnly },
                                            { Property::kDriveMediaRemovable, DeviceProperty::kMediaRemovable },
                                            { Property::kDriveOptical, DeviceProperty::kOptical },
                                            { Property::kDriveOpticalBlank, DeviceProperty::kOpticalBlank },
                                            { Property::kDriveMediaAvailable, DeviceProperty::kMediaAvailable },
                                            { Property::kDriveCanPowerOff, DeviceProperty::kCanPowerOff },
                                            { Property::kDriveEjectable, DeviceProperty::kEjectable },
                                            { Property::kBlockHintIgnore, DeviceProperty::kHintIgnore },
                                            { Property::kBlockCryptoBackingDevice, DeviceProperty::kCryptoBackingDevice },
                                            { Property::kFileSystemMountPoint, DeviceProperty::kMountPoints },
                                            { Property::kDriveMediaCompatibility, DeviceProperty::kMediaCompatibility },
                                            { Property::kEncryptedCleartextDevice, DeviceProperty::kCleartextDevice } };
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

    qCDebug(logDFMBase) << "optical usage persistented: " << datas;
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

        qCDebug(logDFMBase) << "optical usage loaded: " << tag << "\n"
                            << "sizeTotal: " << datas.value(DeviceProperty::kSizeTotal) << "\n"
                            << "sizeUsed: " << datas.value(DeviceProperty::kSizeUsed) << "\n"
                            << "sizeFree: " << datas.value(DeviceProperty::kSizeFree) << "\n"
                            << "mediaType: " << datas.value(DeviceProperty::kOpticalMediaType) << "\n"
                            << "speed: " << datas.value(DeviceProperty::kOpticalWriteSpeed);
    }
}

bool DeviceHelper::checkNetworkConnection(const QString &id)
{
    QUrl url(id);
    if (!(DeviceUtils::isSamba(url) || DeviceUtils::isSftp(url) || DeviceUtils::isFtp(url)))
        return true;

    QString host, port;
    if (NetworkUtils::instance()->parseIp(url.path(), host, port)) {
        if (url.scheme() == "smb" && url.port() == -1) {
            QStringList defaultSmbPorts = { "445", "139" };
            return std::any_of(defaultSmbPorts.cbegin(), defaultSmbPorts.cend(),
                               [host](const QString &port) {
                                   bool connected = NetworkUtils::instance()->checkNetConnection(host, port);
                                   qCDebug(logDFMBase) << "checking network connection of" << host
                                                       << "at" << port
                                                       << connected;
                                   return connected;
                               });
        }
        return NetworkUtils::instance()->checkNetConnection(host, port);
    }

    qCWarning(logDFMBase) << "cannot parse host and port of" << id;
    return true;
}

/* this is used to get a fake protocol device info.
 * if network disconnected, and a network device is already mounted before that
 * then dde-desktop will be stocked in launch process.
 * gio api blocked at query info of protocol device.
 * so if network disconnected, return a fake info and return real info when network resumed.
 * */
QVariantMap DeviceHelper::makeFakeProtocolInfo(const QString &id)
{
    using namespace GlobalServerDefines;
    QString path = QUrl::fromPercentEncoding(id.toLocal8Bit());
    QVariantMap fakeInfo;
    fakeInfo[DeviceProperty::kId] = id;
    fakeInfo[DeviceProperty::kMountPoint] = path.remove(QRegularExpression(R"(^file://)"));
    fakeInfo[DeviceProperty::kDeviceIcon] = "folder-remote";
    fakeInfo["fake"] = true;

    if (DeviceUtils::isSamba(QUrl(path))) {
        QString host, share;
        if (DeviceUtils::parseSmbInfo(path, host, share))
            fakeInfo[DeviceProperty::kDisplayName] = QObject::tr("%1 on %2").arg(share).arg(host);
        else
            fakeInfo[DeviceProperty::kDisplayName] = QObject::tr("Unknown");
    } else {
        QString host, port;
        if (NetworkUtils::instance()->parseIp(QUrl(id).path(), host, port))
            fakeInfo[DeviceProperty::kDisplayName] = host;
        else
            fakeInfo[DeviceProperty::kDisplayName] = QObject::tr("Unknown");
    }

    return fakeInfo;
}
