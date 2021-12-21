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
#include "file/entry/entryfileinfo.h"
#include "dbusservice/global_server_defines.h"
#include "utils/fileutils.h"
#include "utils/universalutils.h"

#include "base/application/application.h"
#include "base/application/settings.h"
#include "base/urlroute.h"

#include <QMenu>

DFMBASE_USE_NAMESPACE

namespace IconName {
const char *const kRootBlock { "drive-harddisk-root" };
const char *const kInnerBlock { "drive-harddisk" };
const char *const kEncryptedInnerBlock { "drive-harddisk-encrypted" };
const char *const kRemovableBlock { "drive-removable-media" };
const char *const kEncryptedRemovableBlock { "drive-removable-media-encrypted" };
const char *const kOpticalBlock { "media-optical" };
const char *const kOpticalBlockExternal { "media-external" };
}   // namespace IconName

namespace AdditionalProperty {
const char *const kClearBlockProperty { "clearBlockDevice" };
const char *const kAliasGroupName { "LocalDiskAlias" };
const char *const kAliasItemName { "Items" };
const char *const kAliasItemUUID { "uuid" };
const char *const kAliasItemAlias { "alias" };
}   // namespace AdditionalProperty

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
    // TODO(xust) more details
    if (datas.value(DeviceProperty::kHintSystem).toBool()) {
        return getNameOrAlias();
    }

    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.value(DeviceProperty::kCleartextDevice).toString().length() == 1) {
            return tr("%1 Encrypted").arg(FileUtils::formatSize(datas.value(DeviceProperty::kSizeTotal).toLongLong()));
        } else {
            return datas.value(AdditionalProperty::kClearBlockProperty).toMap().value(DeviceProperty::kIdLabel).toString();
        }
    }

    QString label = datas.value(DeviceProperty::kIdLabel).toString();
    if (label.isEmpty()) {
        long size = datas.value(DeviceProperty::kSizeTotal).toLongLong();
        return tr("%1 Volume").arg(FileUtils::formatSize(size));
    }
    return label;
}

QIcon BlockEntryFileEntity::icon() const
{
    bool isRootDisk = datas.value(DeviceProperty::kMountpoint).toString() == "/";
    if (isRootDisk)
        return QIcon::fromTheme(IconName::kRootBlock);

    bool isOptical = datas.value(DeviceProperty::kOptical).toBool();
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
        if (datas.value(DeviceProperty::kMountpoint).toString() == "/")
            return EntryFileInfo::EntryOrder::kOrderSysDiskRoot;
        if (0)   // if is Data TODO(xust)
            return EntryFileInfo::EntryOrder::kOrderSysDiskData;
        return EntryFileInfo::EntryOrder::kOrderSysDisks;
    }

    if (datas.value(DeviceProperty::kOptical).toBool()) {
        return EntryFileInfo::EntryOrder::kOrderOptical;
    }

    return EntryFileInfo::EntryOrder::kOrderRemovableDisks;
}

long BlockEntryFileEntity::sizeTotal() const
{
    return getProperty(DeviceProperty::kSizeTotal).toLongLong();
}

long BlockEntryFileEntity::sizeUsage() const
{
    return getProperty(DeviceProperty::kSizeUsed).toLongLong();
}

QString BlockEntryFileEntity::fileSystem() const
{
    if (getProperty(DeviceProperty::kMountpoint).toString().isEmpty())
        return {};

    return getProperty(DeviceProperty::kFilesystem).toString();
}

void BlockEntryFileEntity::refresh()
{
    auto id = QString(DeviceId::kBlockDeviceIdPrefix)
            + entryUrl.path().remove("." + QString(SuffixInfo::kBlock));

    auto reply = UniversalUtils::deviceManager()->QueryBlockDeviceInfo(id, true);
    reply.waitForFinished();
    if (reply.isValid()) {
        datas = reply.value();
        auto clearBlkId = datas.value(DeviceProperty::kCleartextDevice).toString();

        if (datas.value(DeviceProperty::kIsEncrypted).toBool()
            && clearBlkId.length() > 1) {
            // query the real block's info
            auto clearBlkReply = UniversalUtils::deviceManager()->QueryBlockDeviceInfo(clearBlkId, true);
            clearBlkReply.waitForFinished();
            if (clearBlkReply.isValid()) {
                auto clearDatas = clearBlkReply.value();
                datas.insert(AdditionalProperty::kClearBlockProperty, clearDatas);
            } else {
                qDebug() << "cannot get detail info of " << clearBlkId;
            }
        }
    } else {
        qDebug() << "cannot get info of " << id;
    }
}

bool BlockEntryFileEntity::removable()
{
    return datas.value(DeviceProperty::kRemovable).toBool();
}

QMenu *BlockEntryFileEntity::createMenu()
{
    // TODO(xust) may delete later.
    QMenu *menu = new QMenu();
    QAction *act = new QAction(tr("Open"), menu);
    menu->addAction(act);
    act = new QAction(tr("Open in new window"), menu);
    menu->addAction(act);
    act = new QAction(tr("Open in new tab"), menu);
    menu->addAction(act);
    menu->addSeparator();
    act = new QAction(tr("Property"), menu);
    menu->addAction(act);
    return menu;
}

QUrl BlockEntryFileEntity::targetUrl() const
{
    auto mpt = getProperty(DeviceProperty::kMountpoint).toString();
    QUrl target;
    if (mpt.isEmpty())
        return target;
    target.setScheme(dfmbase::SchemeTypes::kFile);
    target.setPath(mpt);
    return target;
}

QString BlockEntryFileEntity::getNameOrAlias() const
{
    // TODO(xust)
    const auto &lst = Application::genericSetting()->value(AdditionalProperty::kAliasGroupName, AdditionalProperty::kAliasItemName).toList();

    QString name;
    for (const QVariant &v : lst) {
        const QVariantMap &map = v.toMap();
        if (map.value(AdditionalProperty::kAliasItemUUID).toString() == datas.value(DeviceProperty::kUUID).toString()) {
            name = map.value(AdditionalProperty::kAliasItemAlias).toString();
            break;
        }
    }

    if (!name.isEmpty())
        return name;

    // get system disk name if there is no alias

    return name;
}

QVariant BlockEntryFileEntity::getProperty(const char *const key) const
{
    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(AdditionalProperty::kClearBlockProperty))
            return datas.value(AdditionalProperty::kClearBlockProperty).toMap().value(key);
        return {};
    }
    return datas.value(key);
}

bool BlockEntryFileEntity::showSizeAndProgress() const
{
    if (getProperty(DeviceProperty::kMountpoint).toString().isEmpty())
        return false;

    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(AdditionalProperty::kClearBlockProperty))
            return true;
        return false;
    }

    return true;
}
