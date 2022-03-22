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
#include "computermodel.h"
#include "views/computerview.h"
#include "utils/computerutils.h"
#include "controller/computercontroller.h"
#include "fileentity/blockentryfileentity.h"

#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/utils/fileutils.h"

#include <QVector>

DPCOMPUTER_BEGIN_NAMESPACE
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
    case Qt::DisplayRole:
        if (item->shape == ComputerItemData::kSplitterItem)
            return item->groupName;
        return item->info ? item->info->displayName() : "";

    case Qt::DecorationRole:
        return item->info ? item->info->fileIcon() : QIcon();

    case kSizeTotalRole:
        return item->info ? QVariant::fromValue<long>(item->info->sizeTotal()) : 0;

    case kSizeUsageRole:
        return item->info ? QVariant::fromValue<long>(item->info->sizeUsage()) : 0;

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
        return item->info ? item->info->suffix() : "";

    case kProgressVisiableRole:
        return item->info ? item->info->showProgress() : false;

    case kTotalSizeVisiableRole:
        return item->info ? item->info->showTotalSize() : false;

    case kUsedSizeVisiableRole:
        return item->info ? item->info->showUsedSize() : false;

    case kDeviceNameMaxLengthRole:
        return DFMBASE_NAMESPACE::FileUtils::supportedMaxLength(item->info ? item->info->extraProperty(DeviceProperty::kFileSystem).toString() : "");

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
    if (items.count() <= index.row())
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
        return item.url == target;
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
        if (item.shape == ComputerItemData::kSplitterItem && item.groupName == group)
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
        QTimer::singleShot(0, this, [this]() {
            view->hideSystemPartitions(ComputerUtils::shouldSystemPartitionHide());
            view->hideLoopPartitions(ComputerUtils::shouldLoopPartitionsHide());
        });
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
        pos = findSplitter(data.groupName);
        if (pos >= 0)
            return;
    }

    pos = findItem(data.url);
    if (pos > 0)   // update the item
        onItemUpdated(data.url);
    else {
        if (shape != ComputerItemData::kSplitterItem && data.info->order() >= EntryFileInfo::kOrderCustom) {
            beginInsertRows(QModelIndex(), items.count() - 1, items.count() - 1);
            items.append(data);
            endInsertRows();
        } else {
            int i = 0;
            for (; i < items.count(); i++) {
                const auto &item = items.at(i);
                if (item.groupId != data.groupId)
                    continue;

                if (ComputerItemWatcher::typeCompare(data, item))
                    break;

                int next = i + 1;
                if (next >= items.count())
                    break;
                const auto &nextItem = items.at(next);
                if (nextItem.groupId != data.groupId) {
                    i = next;
                    break;
                }
            }

            beginInsertRows(QModelIndex(), i, i);
            items.insert(i, data);
            endInsertRows();
        }
    }
}

void ComputerModel::onItemRemoved(const QUrl &url)
{
    int pos = findItem(url);
    if (pos > 0) {
        beginRemoveRows(QModelIndex(), pos, pos);
        items.removeAt(pos);
        endRemoveRows();
        if (items.last().shape == ComputerItemData::kSplitterItem) {
            pos = items.count() - 1;
            beginRemoveRows(QModelIndex(), pos, pos);
            items.removeAt(pos);
            endRemoveRows();
            qDebug() << "remove last orphan group";
        }
    } else {
        qDebug() << "target item not found" << url;
    }
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

    DSB_FM_USE_NAMESPACE
    if (key == DeviceProperty::kIdLabel && !val.toString().isEmpty())
        SideBarService::service()->updateItemName(url, val.toString(), true);
}

DPCOMPUTER_END_NAMESPACE
