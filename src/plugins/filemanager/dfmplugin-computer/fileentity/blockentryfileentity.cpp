/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "blockentryfileentity.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"

#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/urlroute.h"

#include <QMenu>

DFMBASE_USE_NAMESPACE
DPCOMPUTER_USE_NAMESPACE

namespace IconName {
static constexpr char kRootBlock[] { "drive-harddisk-root" };
static constexpr char kInnerBlock[] { "drive-harddisk" };
static constexpr char kEncryptedInnerBlock[] { "drive-harddisk-encrypted" };
static constexpr char kRemovableBlock[] { "drive-removable-media" };
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
        qWarning() << "wrong suffix in" << __FUNCTION__ << "url";
        abort();
    }

    refresh();
}

QString BlockEntryFileEntity::displayName() const
{
    QString label = datas.value(DeviceProperty::kIdLabel).toString();
    long size = datas.value(DeviceProperty::kSizeTotal).toLongLong();

    if (datas.value(DeviceProperty::kHintSystem).toBool())
        return getNameOrAlias();
    else if (datas.value(DeviceProperty::kIsEncrypted).toBool())
        return getNameOfEncrypted();
    else if (datas.value(DeviceProperty::kOpticalDrive).toBool())
        return getNameOfOptical();
    else
        return getDefaultLabel(label, size);
}

QIcon BlockEntryFileEntity::icon() const
{
    bool isRootDisk = datas.value(DeviceProperty::kMountPoint).toString() == "/";
    if (isRootDisk)
        return QIcon::fromTheme(IconName::kRootBlock);

    bool isOptical = datas.value(DeviceProperty::kOptical).toBool() || datas.value(DeviceProperty::kOpticalDrive).toBool();
    bool isRemovable = datas.value(DeviceProperty::kRemovable).toBool();
    if (isOptical)
        return isRemovable ? QIcon::fromTheme(IconName::kOpticalBlockExternal) : QIcon::fromTheme(IconName::kOpticalBlock);

    bool isSysDisk = datas.value(DeviceProperty::kHintSystem).toBool();
    bool isEncrypted = datas.value(DeviceProperty::kIsEncrypted).toBool();
    if (isSysDisk)
        return isEncrypted ? QIcon::fromTheme(IconName::kEncryptedInnerBlock) : QIcon::fromTheme(IconName::kInnerBlock);

    return isEncrypted ? QIcon::fromTheme(IconName::kEncryptedRemovableBlock) : QIcon::fromTheme(IconName::kRemovableBlock);
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

    if (hintIgnore) {
        qWarning() << "block device is ignored by hintIgnore:" << id;
        return false;
    }

    if (!hasFileSystem && !opticalDrive && !isEncrypted) {
        bool removable { qvariant_cast<bool>(datas.value(DeviceProperty::kRemovable)) };
        if (!removable) {   // 满足外围条件的本地磁盘，直接遵循以前的处理直接 continue
            qWarning() << "block device is ignored by wrong removeable set for system disk:" << id;
            return false;
        }
    }

    if (cryptoBackingDevice.length() > 1) {
        qWarning() << "block device is ignored by crypted back device:" << id;
        return false;
    }

    // 是否是设备根节点，设备根节点无须记录
    if (hasPartitionTable) {   // 替换 FileUtils::deviceShouldBeIgnore
        qDebug() << "block device is ignored by parent node:" << id;
        return false;
    }

    // 过滤snap产生的loop设备
    if (isLoopDevice) {
        qDebug() << "block device is ignored by loop device:" << id;
        return false;
    }

    if (hasPartition && hasExtendedPartition) {
        qWarning() << "block device is ignored by extended partion type";
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

void BlockEntryFileEntity::onOpen()
{
    // TODO(xust)
}

EntryFileInfo::EntryOrder BlockEntryFileEntity::order() const
{
    if (datas.value(DeviceProperty::kHintSystem).toBool()) {
        if (datas.value(DeviceProperty::kMountPoint).toString() == "/")
            return EntryFileInfo::EntryOrder::kOrderSysDiskRoot;
        if (datas.value(DeviceProperty::kIdLabel).toString().startsWith("_dde_"))
            return EntryFileInfo::EntryOrder::kOrderSysDiskData;
        return EntryFileInfo::EntryOrder::kOrderSysDisks;
    }

    if (datas.value(DeviceProperty::kOptical).toBool() || datas.value(DeviceProperty::kOpticalDrive).toBool()) {
        return EntryFileInfo::EntryOrder::kOrderOptical;
    }

    return EntryFileInfo::EntryOrder::kOrderRemovableDisks;
}

qint64 BlockEntryFileEntity::sizeTotal() const
{
    return getProperty(DeviceProperty::kSizeTotal).toLongLong();
}

qint64 BlockEntryFileEntity::sizeUsage() const
{
    return getProperty(DeviceProperty::kSizeUsed).toLongLong();
}

void BlockEntryFileEntity::refresh()
{
    auto id = QString(DeviceId::kBlockDeviceIdPrefix)
            + entryUrl.path().remove("." + QString(SuffixInfo::kBlock));

    auto queryInfo = [](const QString &id, bool detail) {
        return DeviceManagerInstance.invokeQueryBlockDeviceInfo(id, detail);
    };

    datas = UniversalUtils::convertFromQMap(queryInfo(id, true));
    auto clearBlkId = datas.value(DeviceProperty::kCleartextDevice).toString();
    if (datas.value(DeviceProperty::kIsEncrypted).toBool() && clearBlkId.length() > 1) {
        auto clearBlkData = queryInfo(clearBlkId, true);
        datas.insert(BlockAdditionalProperty::kClearBlockProperty, clearBlkData);
    }
}

QMenu *BlockEntryFileEntity::createMenu()
{
    QMenu *menu = new QMenu();

    menu->addAction(ContextMenuActionTrs::trOpenInNewWin());
    menu->addAction(ContextMenuActionTrs::trOpenInNewTab());
    menu->addSeparator();

    if (datas.value(DeviceProperty::kHintSystem).toBool()) {
        menu->addAction(ContextMenuActionTrs::trRename());
    } else {
        bool isOpticalDrive = datas.value(DeviceProperty::kOpticalDrive).toBool();
        if (targetUrl().isValid()) {
            menu->addAction(ContextMenuActionTrs::trUnmount());
        } else {
            menu->addAction(ContextMenuActionTrs::trMount());
            if (!isOpticalDrive) {   // optical drive cannot be renamed and formated
                menu->addAction(ContextMenuActionTrs::trRename());
                menu->addAction(ContextMenuActionTrs::trFormat());
            }
        }

        if (isOpticalDrive) {
            if (datas.value(DeviceProperty::kOptical).toBool()
                && datas.value(DeviceProperty::kMedia).toString().contains(QRegularExpression("_r(w|e)")))
                menu->addAction(ContextMenuActionTrs::trErase());
            menu->addAction(ContextMenuActionTrs::trEject());
        }

        menu->addAction(ContextMenuActionTrs::trSafelyRemove());
    }
    menu->addSeparator();

    menu->addAction(ContextMenuActionTrs::trProperties());

    return menu;
}

QUrl BlockEntryFileEntity::targetUrl() const
{
    auto mpt = getProperty(DeviceProperty::kMountPoint).toString();
    QUrl target;
    if (mpt.isEmpty())
        return target;
    target.setScheme(DFMBASE_NAMESPACE::SchemeTypes::kFile);
    target.setPath(mpt);
    return target;
}

bool BlockEntryFileEntity::isAccessable() const
{
    if (datas.value(DeviceProperty::kIsEncrypted).toBool())
        return true;
    return datas.value(DeviceProperty::kHasFileSystem).toBool();
}

bool BlockEntryFileEntity::renamable() const
{
    if (!datas.value(DeviceProperty::kRemovable).toBool()) {
        return true;
    } else {
        if (isAccessable() && datas.value(DeviceProperty::kMountPoint).toString().isEmpty())
            return true;
        return false;
    }
}

QString BlockEntryFileEntity::getNameOrAlias() const
{
    const auto &lst = Application::genericSetting()->value(BlockAdditionalProperty::kAliasGroupName, BlockAdditionalProperty::kAliasItemName).toList();

    for (const QVariant &v : lst) {
        const QVariantMap &map = v.toMap();
        if (map.value(BlockAdditionalProperty::kAliasItemUUID).toString() == datas.value(DeviceProperty::kUUID).toString()) {
            return map.value(BlockAdditionalProperty::kAliasItemAlias).toString();
        }
    }

    QString label = datas.value(DeviceProperty::kIdLabel).toString();
    qlonglong size = datas.value(DeviceProperty::kSizeTotal).toLongLong();

    // get system disk name if there is no alias
    if (datas.value(DeviceProperty::kMountPoint).toString() == "/")
        return tr("System Disk");
    if (datas.value(DeviceProperty::kIdLabel).toString() == "_dde_")
        return tr("Data Disk");
    return getDefaultLabel(label, size);
}

QString BlockEntryFileEntity::getNameOfOptical() const
{
    QString label = datas.value(DeviceProperty::kIdLabel).toString();
    if (!label.isEmpty())
        return label;

    static const std::initializer_list<std::pair<QString, QString>> opticalMedias {
        { "optical", "Optical" },
        { "optical_cd", "CD-ROM" },
        { "optical_cd_r", "CD-R" },
        { "optical_cd_rw", "CD-RW" },
        { "optical_dvd", "DVD-ROM" },
        { "optical_dvd_r", "DVD-R" },
        { "optical_dvd_rw", "DVD-RW" },
        { "optical_dvd_ram", "DVD-RAM" },
        { "optical_dvd_plus_r", "DVD+R" },
        { "optical_dvd_plus_rw", "DVD+RW" },
        { "optical_dvd_plus_r_dl", "DVD+R/DL" },
        { "optical_dvd_plus_rw_dl", "DVD+RW/DL" },
        { "optical_bd", "BD-ROM" },
        { "optical_bd_r", "BD-R" },
        { "optical_bd_re", "BD-RE" },
        { "optical_hddvd", "HD DVD-ROM" },
        { "optical_hddvd_r", "HD DVD-R" },
        { "optical_hddvd_rw", "HD DVD-RW" },
        { "optical_mo", "MO" }
    };
    static const QMap<QString, QString> discMapper(opticalMedias);
    static const QVector<std::pair<QString, QString>> discVector(opticalMedias);

    qlonglong totalSize = datas.value(DeviceProperty::kSizeTotal).toLongLong();

    if (datas.value(DeviceProperty::kOptical).toBool()) {   // medium loaded
        if (datas.value(DeviceProperty::kOpticalBlank).toBool()) {   // show empty disc name
            QString mediaType = datas.value(DeviceProperty::kMedia).toString();
            return tr("Blank %1 Disc").arg(discMapper.value(mediaType, tr("Unknown")));
        } else {
            return getDefaultLabel(label, totalSize);
        }
    } else {   // show drive name, medium is not loaded
        auto medias = datas.value(DeviceProperty::kMediaCompatibility).toStringList();
        QString maxCompatibility;
        for (auto iter = discVector.crbegin(); iter != discVector.crend(); ++iter) {
            if (medias.contains(iter->first))
                return tr("%1 Drive").arg(iter->second);
        }
    }

    return getDefaultLabel(label, totalSize);
}

QString BlockEntryFileEntity::getNameOfEncrypted() const
{
    if (datas.value(DeviceProperty::kCleartextDevice).toString().length() > 1
        && !datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap().isEmpty()) {
        auto clearDevData = datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap();
        QString clearDevLabel = clearDevData.value(DeviceProperty::kIdLabel).toString();
        qlonglong clearDevSize = clearDevData.value(DeviceProperty::kSizeTotal).toLongLong();
        return getDefaultLabel(clearDevLabel, clearDevSize);
    } else {
        return tr("%1 Encrypted").arg(FileUtils::formatSize(datas.value(DeviceProperty::kSizeTotal).toLongLong()));
    }
}

QString BlockEntryFileEntity::getDefaultLabel(const QString &label, qlonglong size) const
{
    if (label.isEmpty())
        return tr("%1 Volume").arg(FileUtils::formatSize(size));
    return label;
}

QVariant BlockEntryFileEntity::getProperty(const char *const key) const
{
    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(BlockAdditionalProperty::kClearBlockProperty))
            return datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap().value(key);
        return {};
    }
    return datas.value(key);
}

bool BlockEntryFileEntity::showSizeAndProgress() const
{
    if (getProperty(DeviceProperty::kMountPoint).toString().isEmpty())
        return false;

    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(BlockAdditionalProperty::kClearBlockProperty))
            return true;
        return false;
    }

    return true;
}
