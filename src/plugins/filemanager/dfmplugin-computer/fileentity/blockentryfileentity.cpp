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
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/base/device/deviceutils.h"
#include "dfm-base/base/urlroute.h"

#include <QMenu>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>

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
    return DeviceUtils::convertSuitableDisplayName(datas);
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

    if (isLoopDevice && !hasFileSystem) {
        qWarning() << "loop device has no filesystem so hide it: " << id;
        return false;
    }

    if (hintIgnore && !isLoopDevice) {
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
    auto id = QString(DeviceId::kBlockDeviceIdPrefix)
            + entryUrl.path().remove("." + QString(SuffixInfo::kBlock));

    datas = UniversalUtils::convertFromQMap(DevProxyMng->queryBlockInfo(id));
    auto clearBlkId = datas.value(DeviceProperty::kCleartextDevice).toString();
    if (datas.value(DeviceProperty::kIsEncrypted).toBool() && clearBlkId.length() > 1) {
        auto clearBlkData = DevProxyMng->queryBlockInfo(id);
        datas.insert(BlockAdditionalProperty::kClearBlockProperty, clearBlkData);
    }
}

QMenu *BlockEntryFileEntity::createMenu()
{
    QMenu *menu = new QMenu();

    menu->addAction(ContextMenuActionTrs::trOpenInNewWin());
    menu->addAction(ContextMenuActionTrs::trOpenInNewTab());
    menu->addSeparator();

    bool isOpticalDrive = datas.value(DeviceProperty::kOpticalDrive).toBool();
    bool isOptical = datas.value(DeviceProperty::kOptical).toBool();
    if (datas.value(DeviceProperty::kHintSystem).toBool()) {
        menu->addAction(ContextMenuActionTrs::trRename());
    } else {
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
            if (isOptical
                && datas.value(DeviceProperty::kMedia).toString().contains(QRegularExpression("_r(w|e)")))
                menu->addAction(ContextMenuActionTrs::trErase());
            menu->addAction(ContextMenuActionTrs::trEject());
        }

        menu->addAction(ContextMenuActionTrs::trSafelyRemove());
    }
    menu->addSeparator();

    menu->addAction(ContextMenuActionTrs::trProperties());

    // disable actions
    if (isOpticalDrive && !isOptical) {
        for (auto act : menu->actions()) {
            if (act->text() != ContextMenuActionTrs::trEject())
                act->setDisabled(true);
        }
    }

    return menu;
}

QUrl BlockEntryFileEntity::targetUrl() const
{
    auto mpt = getProperty(DeviceProperty::kMountPoint).toString();
    QUrl target;
    if (mpt.isEmpty())
        return target;

    // when enter DataDisk, enter Home directory.
    if (QUrl::fromLocalFile(mpt) == QUrl::fromLocalFile("/data")) {
        QString userHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        if (QDir("/data" + userHome).exists())
            return QUrl::fromLocalFile("/data" + userHome);
    }

    target.setScheme(DFMBASE_NAMESPACE::Global::kFile);
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
    if (datas.value(DeviceProperty::kOpticalDrive).toBool())
        return false;
    if (!datas.value(DeviceProperty::kRemovable).toBool()) {
        return true;
    } else {
        if (isAccessable() && datas.value(DeviceProperty::kMountPoint).toString().isEmpty())
            return true;
        return false;
    }
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
