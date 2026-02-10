// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computermodel.h"
#include "views/computerview.h"
#include "utils/computerutils.h"
#include "controller/computercontroller.h"
#include "fileentity/blockentryfileentity.h"

#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-framework/event/event.h>

#include <QVector>
#include <QTimer>

namespace dfmplugin_computer {
using namespace GlobalServerDefines;

ComputerModel::ComputerModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    items = ComputerItemWatcherInstance->getInitedItems();
    initConnect();
}

ComputerModel::~ComputerModel()
{
}

QModelIndex ComputerModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (row >= rowCount() || row < 0)
        return QModelIndex();
    return createIndex(row, column, const_cast<ComputerItemData *>(&items[row]));
}

QModelIndex ComputerModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int ComputerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return items.count();
}

int ComputerModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ComputerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || items.count() <= index.row()) {
        fmWarning() << "ComputerModel::data invalid index row:" << index.row() << "items count:" << items.count();
        return {};
    }

    const auto item = &items[index.row()];
    if (!item) {
        fmCritical() << "ComputerModel::data null item at row:" << index.row();
        return {};
    }

    switch (role) {
    case Qt::DisplayRole: {
        if (item->shape == ComputerItemData::kSplitterItem)
            return item->itemName;
        if (!item->info) {
            fmWarning() << "ComputerModel::data null info for display role at row:" << index.row();
            return "";
        }

        QString &&itemName = item->info->displayName();
        if (itemName != item->itemName)
            item->itemName = itemName;
        return itemName;
    }

    case Qt::DecorationRole:
        return item->info ? item->info->fileIcon() : QIcon();

    case kSizeTotalRole:
        return item->info ? QVariant::fromValue<qulonglong>(item->info->sizeTotal()) : 0;

    case kSizeUsageRole:
        return item->info ? QVariant::fromValue<qulonglong>(item->info->sizeUsage()) : 0;

    case kFileSystemRole: {
        if (!item->info) {
            fmWarning() << "ComputerModel::data null info for filesystem role at row:" << index.row();
            return "";
        }
        if (item->info->targetUrl().isValid()) {
            auto properties = item->info->extraProperties();
            if (properties.value(DeviceProperty::kIsEncrypted).toBool()
                && properties.contains(BlockAdditionalProperty::kClearBlockProperty))
                return properties.value(BlockAdditionalProperty::kClearBlockProperty).toHash().value(DeviceProperty::kFileSystem).toString();
            return item->info->extraProperty(DeviceProperty::kFileSystem).toString();
        }
        return "";
    }

    case kRealUrlRole:
        return item->info ? item->info->targetUrl() : QUrl();

    case kDeviceUrlRole:
        return item->url;

    case kSuffixRole:
        return item->info ? item->info->nameOf(NameInfoType::kSuffix) : "";

    case kProgressVisiableRole:
        return item->info ? item->info->showProgress() : false;

    case kTotalSizeVisiableRole:
        return item->info ? item->info->showTotalSize() : false;

    case kUsedSizeVisiableRole:
        return item->info ? item->info->showUsedSize() : false;

    case kDeviceNameMaxLengthRole: {
        QString realFs;
        if (auto inf = item->info) {
            if (!inf->extraProperty(DeviceProperty::kIsEncrypted).toBool()) {
                realFs = inf->extraProperty(DeviceProperty::kFileSystem).toString();
            } else {
                const auto &clearDevId = inf->extraProperty(DeviceProperty::kCleartextDevice).toString();
                if (clearDevId != "/") {
                    // NOTE(xust): this code block WILL/SHOULD only be invoked while renaming
                    // and only once in every rename operation.
                    // and will not deoptimize the performance.
                    // and will only be invoked if device is encrypted.
                    const auto &clearDevData = DevProxyMng->queryBlockInfo(clearDevId);
                    realFs = clearDevData.value(DeviceProperty::kFileSystem).toString();
                }
            }
        }
        return DFMBASE_NAMESPACE::FileUtils::supportedMaxLength(realFs);
    }

    case kItemShapeTypeRole:
        return item->shape;

    case kActionListRole:
        return "";

    case kItemIsEditingRole:
        return item->isEditing;

    case kDeviceIsEncryptedRole:
        return item->info ? item->info->extraProperty(DeviceProperty::kIsEncrypted).toBool() : false;

    case kDeviceIsUnlockedRole: {
        bool isUnlocked = false;
        if (item->info)
            isUnlocked = item->info->extraProperty(DeviceProperty::kCleartextDevice).toString().length() > 1;
        return isUnlocked;
    }

    case kDeviceClearDevIdRole:
        return item->info ? item->info->extraProperty(DeviceProperty::kCleartextDevice).toString() : "";

    case kDeviceDescriptionRole:
        return item->info ? item->info->description() : "";

    case kDisplayNameIsElidedRole:
        return item->isElided;

    case kEditDisplayTextRole:
        return item->info ? item->info->editDisplayText() : "";

    case kItemVisibleRole:
        return item->isVisible;

    default:
        return {};
    }
}

bool ComputerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    if (row < 0 || row >= items.count()) {
        fmWarning() << "ComputerModel::setData invalid row:" << row << "items count:" << items.count();
        return false;
    }

    auto &item = items[row];
    if (role == Qt::EditRole) {
        if (!item.info || !item.info->renamable()) {
            fmWarning() << "ComputerModel::setData item not renamable at row:" << row;
            return false;
        }

        ComputerControllerInstance->doRename(0, item.url, value.toString());
        return true;
    } else if (role == DataRoles::kItemIsEditingRole) {
        item.isEditing = value.toBool();
        return true;
    } else if (role == DataRoles::kDisplayNameIsElidedRole) {
        item.isElided = value.toBool();
        return true;
    } else if (role == DataRoles::kItemVisibleRole) {
        item.isVisible = value.toBool();
        return true;
    }
    return false;
}

Qt::ItemFlags ComputerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemFlag::ItemNeverHasChildren;
    if (!index.isValid() || items.count() <= index.row())
        return flags;

    if (index.data(kItemShapeTypeRole) != ComputerItemData::kSplitterItem) {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    const auto &data = items[index.row()];
    if (data.info && data.info->renamable())
        flags |= Qt::ItemIsEditable;

    return flags;
}

int ComputerModel::findItem(const QUrl &target)
{
    auto iter = std::find_if(items.cbegin(), items.cend(), [=](const ComputerItemData &item) {
        return UniversalUtils::urlEquals(item.url, target);
    });
    if (iter != items.cend())
        return iter - items.cbegin();
    return -1;
}

int ComputerModel::findItemByClearDeviceId(const QString &id)
{
    auto iter = std::find_if(items.cbegin(), items.cend(), [=](const ComputerItemData &item) {
        return item.info ? item.info->extraProperty(DeviceProperty::kCleartextDevice).toString() == id : false;
    });
    if (iter != items.cend())
        return iter - items.cbegin();
    return -1;
}

int ComputerModel::findSplitter(const QString &group)
{
    auto iter = std::find_if(items.cbegin(), items.cend(), [=](const ComputerItemData &item) {
        if (item.shape == ComputerItemData::kSplitterItem && item.itemName == group)
            return true;
        return false;
    });
    if (iter != items.cend())
        return iter - items.cbegin();
    return -1;
}

ComputerItemData ComputerModel::findItemData(const QUrl &target)
{
    auto iter = std::find_if(items.cbegin(), items.cend(), [=](const ComputerItemData &item) {
        return UniversalUtils::urlEquals(item.url, target);
    });
    if (iter != items.cend())
        return *iter;
    return {};
}

const QList<ComputerItemData> &ComputerModel::itemList()
{
    return items;
}

void ComputerModel::initConnect()
{
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::itemQueryFinished, this, [this](const ComputerDataList &datas) {
        this->beginResetModel();
        items = datas;
        this->endResetModel();
        emit requestHandleItemVisible();
    });
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::itemAdded, this, &ComputerModel::onItemAdded);
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::itemRemoved, this, &ComputerModel::onItemRemoved);

    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::itemUpdated, this, &ComputerModel::onItemUpdated);
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::itemSizeChanged, this, &ComputerModel::onItemSizeChanged);
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::itemPropertyChanged, this, &ComputerModel::onItemPropertyChanged);
}

void ComputerModel::onItemAdded(const ComputerItemData &data)
{
    ComputerItemData::ShapeType shape = data.shape;
    int pos = -1;
    if (shape == ComputerItemData::kSplitterItem) {
        pos = findSplitter(data.itemName);
        if (pos >= 0) {
            fmDebug() << "ComputerModel::onItemAdded splitter already exists:" << data.itemName;
            return;
        }
    }

    pos = findItem(data.url);
    fmInfo() << "item added: devUrl = " << data.url << ",pos = " << pos;   // log for bug:#182939
    if (pos > 0) {   // update the item
        fmDebug() << "ComputerModel::onItemAdded updating existing item at position:" << pos;
        onItemUpdated(data.url);
    } else {
        if (shape == ComputerItemData::kSplitterItem) {
            fmDebug() << "ComputerModel::onItemAdded adding new splitter group:" << data.itemName;
            addGroup(data);
            return;
        }

        int row = 0;
        for (; row < items.count(); row++) {
            const auto &item = items.at(row);
            if (item.groupId != data.groupId)
                continue;

            if (ComputerItemWatcher::typeCompare(data, item))
                break;

            int next = row + 1;
            if (next >= items.count()   // when search at end OR search at end of the group, append item to last item of the group.
                || items.at(next).groupId != data.groupId) {
                row = next;
                break;
            }
        }

        beginInsertRows(QModelIndex(), row, row);
        if (items.size() < row || row < 0)
            items.append(data);
        else
            items.insert(row, data);
        endInsertRows();
    }

    // for filter the native disks hided by main setting panel
    // and when disk-manager/partition-editor opened and closed,
    // the itemRemoved/Added signals are emitted
    // and these newcomming items should be filtered
    emit requestHandleItemVisible();
}

void ComputerModel::onItemRemoved(const QUrl &url)
{
    int pos = findItem(url);
    if (pos > 0) {
        fmInfo() << "item removed: " << url << ",pos = " << pos;   // log for bug:#224925

        emit requestClearSelection(url);

        beginRemoveRows(QModelIndex(), pos, pos);
        items.removeAt(pos);
        endRemoveRows();
        removeOrphanGroup();
    } else {
        fmDebug() << "target item not found" << url;
        return;
    }

    emit requestHandleItemVisible();
}

void ComputerModel::onItemUpdated(const QUrl &url)
{
    int pos = findItem(url);
    if (pos > 0) {
        updateItemInfo(pos);
    } else {
        pos = findItemByClearDeviceId(ComputerUtils::getBlockDevIdByUrl(url));
        if (pos > 0) {
            updateItemInfo(pos);
            return;
        }
        fmDebug() << "target item not found" << url;
    }
}

void ComputerModel::updateItemInfo(int pos)
{
    if (pos < 0 || pos >= items.count()) {
        fmWarning() << "ComputerModel::updateItemInfo invalid position:" << pos << "items count:" << items.count();
        return;
    }

    auto &info = items.at(pos);
    QString oldName = info.info->displayName();
    info.info->refresh();
    emit requestUpdateIndex(this->index(pos, 0));
}

void ComputerModel::onItemSizeChanged(const QUrl &url, qlonglong total, qlonglong free)
{
    bool foundEncryptedDev = false;
    int pos = findItem(url);
    if (pos < 0 || pos >= items.count()) {
        pos = findItemByClearDeviceId(ComputerUtils::getBlockDevIdByUrl(url));
        foundEncryptedDev = true;
    }
    if (pos < 0 || pos >= items.count())
        return;

    auto &info = items.at(pos);
    if (!info.info)
        return;

    auto entryInfo = info.info;

    if (foundEncryptedDev) {
        auto clearDevInfo = entryInfo->extraProperty(BlockAdditionalProperty::kClearBlockProperty).toHash();
        clearDevInfo[DeviceProperty::kSizeTotal] = total;
        clearDevInfo[DeviceProperty::kSizeFree] = free;
        clearDevInfo[DeviceProperty::kSizeUsed] = total - free;
        entryInfo->setExtraProperty(BlockAdditionalProperty::kClearBlockProperty, clearDevInfo);
    } else {
        entryInfo->setExtraProperty(DeviceProperty::kSizeTotal, total);
        entryInfo->setExtraProperty(DeviceProperty::kSizeFree, free);
        entryInfo->setExtraProperty(DeviceProperty::kSizeUsed, total - free);
    }
    emit requestUpdateIndex(this->index(pos, 0));
}

void ComputerModel::onItemPropertyChanged(const QUrl &url, const QString &key, const QVariant &val)
{
    bool foundEncryptedDev = false;
    int pos = findItem(url);
    if (pos < 0 || pos >= items.count()) {
        pos = findItemByClearDeviceId(ComputerUtils::getBlockDevIdByUrl(url));
        foundEncryptedDev = true;
    }
    if (pos < 0 || pos >= items.count()) {
        fmWarning() << "ComputerModel::onItemPropertyChanged item not found for URL:" << url;
        return;
    }

    auto &info = items.at(pos);
    if (!info.info) {
        fmWarning() << "ComputerModel::onItemPropertyChanged null info at position:" << pos;
        return;
    }

    auto entryInfo = info.info;

    if (foundEncryptedDev) {
        auto clearDevInfo = entryInfo->extraProperty(BlockAdditionalProperty::kClearBlockProperty).toHash();
        clearDevInfo[key] = val;
        if (key == DeviceProperty::kMountPoints) {
            auto mpts = val.toStringList();
            clearDevInfo[DeviceProperty::kMountPoint] = mpts.isEmpty() ? "" : mpts.first();
        }
        entryInfo->setExtraProperty(BlockAdditionalProperty::kClearBlockProperty, clearDevInfo);
    } else {
        entryInfo->setExtraProperty(key, val);
        if (key == DeviceProperty::kMountPoints) {
            auto mpts = val.toStringList();
            entryInfo->setExtraProperty(DeviceProperty::kMountPoint, mpts.isEmpty() ? "" : mpts.first());
        }
    }
    emit requestUpdateIndex(this->index(pos, 0));

    if (key == DeviceProperty::kIdLabel && !val.toString().isEmpty()) {
        QVariantMap map {
            { "Property_Key_DisplayName", val.toString() },
            { "Property_Key_Editable", true }
        };
        dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
    }
}

/*!
 * \brief ComputerModel::addGroup some preset group should be inserted in fixed position.
 * \param data
 */
void ComputerModel::addGroup(const ComputerItemData &data)
{
    if (data.shape != ComputerItemData::kSplitterItem) {
        fmWarning() << "ComputerModel::addGroup called with non-splitter item:" << data.url;
        return;
    }

    const QString &name = data.itemName;
    if (name == ComputerItemWatcher::userDirGroup()) {   // insert at 0
        beginInsertRows(QModelIndex(), 0, 0);
        items.insert(0, data);
        endInsertRows();
    } else if (name == ComputerItemWatcher::diskGroup()) {   // insert after last userdir item
        int userDirId = ComputerItemWatcherInstance->getGroupId(ComputerItemWatcher::userDirGroup());
        int insertAt { 0 };
        for (int i = 0; i < items.count(); i++) {
            if (items.at(i).groupId != userDirId)
                break;
            else
                insertAt = i + 1;
        }
        beginInsertRows(QModelIndex(), insertAt, insertAt);

        if (insertAt > items.size() || insertAt < 0)
            items.append(data);
        else
            items.insert(insertAt, data);

        endInsertRows();
    } else {   // append it, maybe someday vault will take the 3rd position.
        beginInsertRows(QModelIndex(), items.count(), items.count());
        items.append(data);
        endInsertRows();
    }
}

void ComputerModel::removeOrphanGroup()
{
    QList<int> aboutToRemovedGroup;
    bool foundGroup { false };
    int i = 0;
    for (; i < items.count(); i++) {
        if (items.at(i).shape == ComputerItemData::kSplitterItem) {
            if (foundGroup)
                aboutToRemovedGroup.append(i - 1);
            foundGroup = true;
        } else {
            foundGroup = false;
        }
    }
    if (foundGroup)
        aboutToRemovedGroup.append(i - 1);

    for (int i = aboutToRemovedGroup.count() - 1; i >= 0; i--) {
        int removeAt { aboutToRemovedGroup.at(i) };
        auto groupName = items.at(removeAt).itemName;
        auto groupRemoved = ComputerItemWatcherInstance->removeGroup(groupName);
        fmInfo() << groupName << "removed? (true if group exists.)" << groupRemoved;

        beginRemoveRows(QModelIndex(), removeAt, removeAt);
        items.removeAt(removeAt);
        endRemoveRows();
    }
}

}
