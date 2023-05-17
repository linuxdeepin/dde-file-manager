// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
    view = qobject_cast<ComputerView *>(parent);
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
    if (items.count() <= index.row())
        return {};
    const auto item = &items[index.row()];
    if (!item)
        return {};

    switch (role) {
    case Qt::DisplayRole: {
        if (item->shape == ComputerItemData::kSplitterItem)
            return item->itemName;
        if (!item->info)
            return "";

        QString &&itemName = item->info->displayName();
        if (itemName != item->itemName) {
            QVariantMap map {
                { "Property_Key_DisplayName", itemName },
                { "Property_Key_Editable", item->info->renamable() }
            };
            dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", item->info->urlOf(UrlInfoType::kUrl), map);
            item->itemName = itemName;
        }
        return itemName;
    }

    case Qt::DecorationRole:
        return item->info ? item->info->fileIcon() : QIcon();

    case kSizeTotalRole:
        return item->info ? QVariant::fromValue<qulonglong>(item->info->sizeTotal()) : 0;

    case kSizeUsageRole:
        return item->info ? QVariant::fromValue<qulonglong>(item->info->sizeUsage()) : 0;

    case kFileSystemRole: {
        if (!item->info)
            return "";
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

    default:
        return {};
    }
}

bool ComputerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    if (row < 0 || row >= items.count())
        return false;

    auto &item = items[row];
    if (role == Qt::EditRole) {
        if (!item.info || !item.info->renamable())
            return false;
        ComputerControllerInstance->doRename(0, item.url, value.toString());
        return true;
    } else if (role == DataRoles::kItemIsEditingRole) {
        item.isEditing = value.toBool();
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

void ComputerModel::initConnect()
{
    connect(ComputerItemWatcherInstance, &ComputerItemWatcher::itemQueryFinished, this, [this](const ComputerDataList &datas) {
        this->beginResetModel();
        items = datas;
        this->endResetModel();
        view->handlePartitionsVisiable();
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
        if (pos >= 0)
            return;
    }

    pos = findItem(data.url);
    qInfo() << "devUrl = " << data.url << ",pos = " << pos;   // log for bug:#182939
    if (pos > 0) {   // update the item
        onItemUpdated(data.url);
    } else {
        if (shape == ComputerItemData::kSplitterItem) {
            addGroup(data);
        } else {
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
            items.insert(row, data);
            endInsertRows();
        }
    }

    // for filter the native disks hided by main setting panel
    // and when disk-manager/partition-editor opened and closed,
    // the itemRemoved/Added signals are emitted
    // and these newcomming items should be filtered
    view->handlePartitionsVisiable();
}

void ComputerModel::onItemRemoved(const QUrl &url)
{
    int pos = findItem(url);
    if (pos > 0) {
        if (view->selectedUrlList().contains(url))
            //        view->clearSelection(); // NOTE: this do not work, might be a bug in QT
            view->setCurrentIndex(QModelIndex());   // NOTE: and this works good.

        beginRemoveRows(QModelIndex(), pos, pos);
        items.removeAt(pos);
        endRemoveRows();
        removeOrphanGroup();
    } else {
        qDebug() << "target item not found" << url;
    }

    view->handlePartitionsVisiable();
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
        qDebug() << "target item not found" << url;
    }
}

void ComputerModel::updateItemInfo(int pos)
{
    if (pos < 0 || pos >= items.count())
        return;
    auto &info = items.at(pos);
    QString oldName = info.info->displayName();
    info.info->refresh();
    view->update(this->index(pos, 0));
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
    view->update(this->index(pos, 0));
}

void ComputerModel::onItemPropertyChanged(const QUrl &url, const QString &key, const QVariant &val)
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
    view->update(this->index(pos, 0));

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
    if (data.shape != ComputerItemData::kSplitterItem)
        return;

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
        beginRemoveRows(QModelIndex(), removeAt, removeAt);
        items.removeAt(removeAt);
        endRemoveRows();
    }
}

}
