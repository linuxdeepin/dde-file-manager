// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blockentryfileentity.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>

#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QTimer>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

namespace IconName {
static constexpr char kRootBlock[] { "drive-harddisk-root" };
static constexpr char kInnerBlock[] { "drive-harddisk" };
static constexpr char kEncryptedInnerBlock[] { "drive-harddisk-encrypted" };
static constexpr char kRemovableBlock[] { "drive-removable-media-usb" };
static constexpr char kEncryptedRemovableBlock[] { "drive-removable-media-encrypted" };
static constexpr char kOpticalBlock[] { "media-optical" };
static constexpr char kOpticalBlockExternal[] { "media-external" };
}   // namespace IconName

using namespace GlobalServerDefines;

/*!
 * \class BlockEntryFileEntity
 * \brief class that present block devices
 */
BlockEntryFileEntity::BlockEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    if (!url.path().endsWith(SuffixInfo::kBlock)) {
        fmCritical() << "Invalid block device URL suffix:" << url;
        abort();
    }

    auto refreshInfo = [this](const QString &id) {
        if (id == ComputerUtils::getBlockDevIdByUrl(entryUrl))
            refresh();
    };

    connect(DevProxyMng, &DeviceProxyManager::blockDevMounted, this, refreshInfo);
    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, refreshInfo);

    loadDiskInfo();
}

QString BlockEntryFileEntity::displayName() const
{
    if (datas.contains(WinVolTagKeys::kWinLabel))
        return datas.value(WinVolTagKeys::kWinLabel).toString();
    return DeviceUtils::convertSuitableDisplayName(datas);
}

QIcon BlockEntryFileEntity::icon() const
{
    // NOTE(xust): removable/hintSystem is not always correct in some certain hardwares.
    // CanPowerOff can be used to determind whether a device is builtin device.
    bool canPowerOff = datas.value(DeviceProperty::kCanPowerOff).toBool();
    bool isEncrypted = datas.value(DeviceProperty::kIsEncrypted).toBool();
    switch (order()) {
    case DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderSysDiskRoot:
        return QIcon::fromTheme(IconName::kRootBlock);
    case DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderSysDiskData:
    case DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderSysDisks:
        return isEncrypted
                ? QIcon::fromTheme(IconName::kEncryptedInnerBlock)
                : QIcon::fromTheme(IconName::kInnerBlock);
    case DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderOptical:
        return canPowerOff
                ? QIcon::fromTheme(IconName::kOpticalBlockExternal)
                : QIcon::fromTheme(IconName::kOpticalBlock);
    case DFMBASE_NAMESPACE::AbstractEntryFileEntity::EntryOrder::kOrderRemovableDisks:
        return isEncrypted
                ? QIcon::fromTheme(IconName::kEncryptedRemovableBlock)
                : QIcon::fromTheme(IconName::kRemovableBlock);
    default:
        break;
    }
    return QIcon::fromTheme(IconName::kRemovableBlock);
}

bool BlockEntryFileEntity::exists() const
{
    const QString &&id = datas.value(DeviceProperty::kId).toString();
    bool hintIgnore { qvariant_cast<bool>(datas.value(DeviceProperty::kHintIgnore)) };
    bool hasFileSystem { qvariant_cast<bool>(datas.value(DeviceProperty::kHasFileSystem)) };
    bool opticalDrive { qvariant_cast<bool>(datas.value(DeviceProperty::kOpticalDrive)) };
    bool isEncrypted { qvariant_cast<bool>(datas.value(DeviceProperty::kIsEncrypted)) };
    QString cryptoBackingDevice { qvariant_cast<QString>(datas.value(DeviceProperty::kCryptoBackingDevice)) };
    bool hasPartitionTable { qvariant_cast<bool>(datas.value(DeviceProperty::kHasPartitionTable)) };
    bool hasPartition { qvariant_cast<bool>(datas.value(DeviceProperty::kHasPartition)) };
    bool hasExtendedPartition { qvariant_cast<bool>(datas.value(DeviceProperty::kHasExtendedPatition)) };
    bool isLoopDevice { qvariant_cast<bool>(datas.value(DeviceProperty::kIsLoopDevice)) };

    QString msg { "blockdevice is ignored: " };

    if (isLoopDevice && !hasFileSystem) {
        fmInfo() << msg << "loop device without filesystem." << id;
        return false;
    }

    if (hintIgnore && !isLoopDevice) {
        fmInfo() << msg << "hintIgnore is TRUE:" << id;
        return false;
    }

    if (!hasFileSystem && !opticalDrive && !isEncrypted) {
        //        bool removable { qvariant_cast<bool>(datas.value(DeviceProperty::kRemovable)) };
        //        if (!removable) {   // 满足外围条件的本地磁盘，直接遵循以前的处理直接 continue
        fmInfo() << msg << "no fs, no optical, no encrypted." << id;
        return false;
        //        }
    }

    if (isEncrypted) {
        QVariantHash clearInfo = datas.value(BlockAdditionalProperty::kClearBlockProperty).toHash();
        if (clearInfo.value(DeviceProperty::kFileSystem).toString() == "LVM2_member") {
            fmInfo() << msg << "lvm group should not be displayed" << id;
            return false;
        }
    }

    if (cryptoBackingDevice.length() > 1) {
        fmInfo() << msg << "decrypted cleartext device." << id;
        return false;
    }

    // 是否是设备根节点，设备根节点无须记录
    //    if (hasPartitionTable) {   // 替换 FileUtils::deviceShouldBeIgnore
    //        fmInfo() << msg << "device with a partition table." << id;
    //        return false;
    //    }

    if (hasPartition && hasExtendedPartition) {
        fmInfo() << msg << "device with extended partition." << id;
        return false;
    }

    quint64 blkSize = { qvariant_cast<quint64>(datas.value(DeviceProperty::kSizeTotal)) };
    if (blkSize < 1024 && !opticalDrive && !hasFileSystem) {
        fmInfo() << msg << "tiny size." << blkSize << id;
        return false;
    }

    return true;
}

bool BlockEntryFileEntity::showProgress() const
{
    return showSizeAndProgress();
}

bool BlockEntryFileEntity::showTotalSize() const
{
    return showSizeAndProgress();
}

bool BlockEntryFileEntity::showUsageSize() const
{
    return showSizeAndProgress();
}

AbstractEntryFileEntity::EntryOrder BlockEntryFileEntity::order() const
{
    // NOTE(xust): removable/hintSystem is not always correct in some certain hardwares.
    auto clearInfo = datas.value(BlockAdditionalProperty::kClearBlockProperty, QVariantHash()).toHash();
    if (DeviceUtils::isSystemDisk(clearInfo.isEmpty() ? datas : clearInfo))
        return AbstractEntryFileEntity::EntryOrder::kOrderSysDiskRoot;

    if (DeviceUtils::isDataDisk(clearInfo.isEmpty() ? datas : clearInfo))
        return AbstractEntryFileEntity::EntryOrder::kOrderSysDiskData;

    if (datas.value(DeviceProperty::kOptical).toBool()
        || datas.value(DeviceProperty::kOpticalDrive).toBool())
        return AbstractEntryFileEntity::EntryOrder::kOrderOptical;

    bool canPowerOff = datas.value(DeviceProperty::kCanPowerOff).toBool();
    if (canPowerOff && !isSiblingOfRoot())
        return AbstractEntryFileEntity::EntryOrder::kOrderRemovableDisks;

    return AbstractEntryFileEntity::EntryOrder::kOrderSysDisks;
}

quint64 BlockEntryFileEntity::sizeTotal() const
{
    return getProperty(DeviceProperty::kSizeTotal).toULongLong();
}

quint64 BlockEntryFileEntity::sizeUsage() const
{
    return getProperty(DeviceProperty::kSizeUsed).toULongLong();
}

void BlockEntryFileEntity::refresh()
{
    loadDiskInfo();
}

QUrl BlockEntryFileEntity::targetUrl() const
{
    return mountPoint();
}

bool BlockEntryFileEntity::isAccessable() const
{
    if (datas.value(DeviceProperty::kIsEncrypted).toBool())
        return true;
    return datas.value(DeviceProperty::kHasFileSystem).toBool();
}

bool BlockEntryFileEntity::renamable() const
{
    if (datas.value(DeviceProperty::kOpticalDrive).toBool()) {
        fmDebug() << "Device not renamable (optical drive):" << entryUrl;
        return false;
    }

    if (datas.value(DeviceProperty::kIsEncrypted).toBool()
        && datas.value(DeviceProperty::kCleartextDevice).toString() == "/") {
        fmDebug() << "Device not renamable (encrypted root device):" << entryUrl;
        return false;
    }

    if (datas.value(DeviceProperty::kIsLoopDevice, false).toBool()) {
        fmDebug() << "Device not renamable (loop device):" << entryUrl;
        return false;
    }

    return isAccessable();
}

QVariant BlockEntryFileEntity::getProperty(const char *const key) const
{
    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(BlockAdditionalProperty::kClearBlockProperty))
            return datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap().value(key);
        fmDebug() << "No clear block property available for encrypted device:" << entryUrl;
        return {};
    }
    return datas.value(key);
}

bool BlockEntryFileEntity::showSizeAndProgress() const
{
    if (getProperty(DeviceProperty::kMountPoint).toString().isEmpty()) {
        fmDebug() << "No mount point, not showing size/progress for device:" << entryUrl;
        return false;
    }

    if (getProperty(DeviceProperty::kOpticalDrive).toBool()
        && !getProperty(DeviceProperty::kMediaAvailable).toBool()) {
        fmDebug() << "Optical drive without media, not showing size/progress for device:" << entryUrl;
        return false;
    }

    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(BlockAdditionalProperty::kClearBlockProperty)) {
            fmDebug() << "Encrypted device with clear property, showing size/progress for device:" << entryUrl;
            return true;
        }
        fmDebug() << "Encrypted device without clear property, not showing size/progress for device:" << entryUrl;
        return false;
    }

    return true;
}

QUrl BlockEntryFileEntity::mountPoint() const
{
    const auto &mptList = getProperty(DeviceProperty::kMountPoints).toStringList();
    QUrl target;

    if (mptList.isEmpty()) {
        fmDebug() << "No mount points found for device:" << entryUrl;
        return target;
    }

    if (DeviceUtils::isSystemDisk(datas))
        return QUrl::fromLocalFile(QDir::rootPath());

    // when enter DataDisk, enter Home directory.
    for (const auto &mpt : mptList) {
        if (DeviceUtils::isDataDisk(datas)) {
            const QString &userHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            const QString &homeBindPath = FileUtils::bindPathTransform(userHome, true);
            if (userHome != homeBindPath && homeBindPath.startsWith(mpt))
                return QUrl::fromLocalFile(homeBindPath);
        }
    }

    target.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
    target.setPath(mptList.first());
    return target;
}

void BlockEntryFileEntity::loadDiskInfo()
{
    auto id = QString(DeviceId::kBlockDeviceIdPrefix)
            + entryUrl.path().remove("." + QString(SuffixInfo::kBlock));

    datas = UniversalUtils::convertFromQMap(DevProxyMng->queryBlockInfo(id, true));
    auto clearBlkId = datas.value(DeviceProperty::kCleartextDevice).toString();
    if (datas.value(DeviceProperty::kIsEncrypted).toBool() && clearBlkId.length() > 1) {
        auto clearBlkData = DevProxyMng->queryBlockInfo(clearBlkId, true);
        datas.insert(BlockAdditionalProperty::kClearBlockProperty, clearBlkData);
    }

    if (mountPoint().isValid()) {
        fmDebug() << "Device is mounted, loading Windows volume tag for:" << entryUrl;
        loadWindowsVoltag();
    } else {
        fmDebug() << "Device not mounted, resetting Windows volume tag for:" << entryUrl;
        resetWindowsVolTag();
    }
}

void BlockEntryFileEntity::loadWindowsVoltag()
{
    static const QString &kWinVolInfoConfig = "/UOSICON/diskinfo.json";
    static const QString &kDiskInfoKey = "DISKINFO";
    static const QString &kUUIDKey = "uuid";
    static const QString &kDriveKey = "drive";
    static const QString &kLabelKey = "label";

    QUrl cfgUrl = QUrl::fromLocalFile(mountPoint().path() + kWinVolInfoConfig);

    DFMIO::DFile f(cfgUrl);
    if (!f.exists()) {
        fmDebug() << "Windows volume config file does not exist:" << cfgUrl;
        return;
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (doc.isNull() || err.error != QJsonParseError::NoError) {
        fmWarning() << "Cannot parse Windows volume config file:" << cfgUrl << "error:" << err.errorString();
        return;
    }

    if (doc.isObject()) {
        auto obj = doc.object();
        if (obj.contains(kDiskInfoKey) && obj.value(kDiskInfoKey).isArray()) {
            auto arr = obj.value(kDiskInfoKey).toArray();
            for (int i = 0; i < arr.size(); ++i) {
                auto itemObj = arr[i].toObject();

                if (itemObj.contains(kUUIDKey))
                    datas.insert(WinVolTagKeys::kWinUUID, itemObj.value(kUUIDKey).toString());
                if (itemObj.contains(kDriveKey))
                    datas.insert(WinVolTagKeys::kWinDrive, itemObj.value(kDriveKey).toString());
                if (itemObj.contains(kLabelKey))
                    datas.insert(WinVolTagKeys::kWinLabel, itemObj.value(kLabelKey).toString());
            }
        }
    } else {
        fmWarning() << "Windows volume config file is not a JSON object:" << cfgUrl;
    }
}

void BlockEntryFileEntity::resetWindowsVolTag()
{
    datas.remove(WinVolTagKeys::kWinUUID);
    datas.remove(WinVolTagKeys::kWinDrive);
    datas.remove(WinVolTagKeys::kWinLabel);
}

bool BlockEntryFileEntity::isSiblingOfRoot() const
{
    return DeviceUtils::isSiblingOfRoot(this->datas);
}
