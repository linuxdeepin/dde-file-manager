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
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/utils/fileutils.h"

#include <QVector>

DPCOMPUTER_BEGIN_NAMESPACE

ComputerModel::ComputerModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    view = qobject_cast<ComputerView *>(parent);
    items = ComputerItemWatcherIns->items();
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
    // TODO(xust)
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

    case kFileSystemRole:
        return item->info ? item->info->fileSystem() : "";

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
        return dfmbase::FileUtils::supportedMaxLength(item->info ? item->info->fileSystem() : "");

    case kItemShapeTypeRole:
        return item->shape;

    case kActionListRole:
        return "";

    case kItemIsEditing:
        return item->isEditing;

    case kDeviceIsEncrypted:
        return item->info ? item->info->isEncrypted() : false;

    case kDeviceIsUnlocked:
        return item->info ? item->info->isUnlocked() : false;

    case kDeviceClearDevId:
        return item->info ? item->info->clearDeviceId() : "";

    default:
        return {};
    }
}

bool ComputerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // TODO(xust)
    int row = index.row();
    if (row < 0 || row >= items.count())
        return false;

    const auto &item = items.at(row);
    if (role == Qt::EditRole) {
        if (!item.info || !item.info->renamable())
            return false;
        // TODO(xust) DO RENAME HERE
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
        return item.info ? item.info->clearDeviceId() == id : false;
    });
    if (iter != items.cend())
        return iter - items.cbegin();
    return -1;
}

void ComputerModel::initConnect()
{
    connect(ComputerItemWatcherIns, &ComputerItemWatcher::itemAdded, this, &ComputerModel::onItemAdded);
    connect(ComputerItemWatcherIns, &ComputerItemWatcher::itemRemoved, this, &ComputerModel::onItemRemoved);

    // TODO(xust) find a way to update the property of devices, not just refresh it.
    connect(ComputerItemWatcherIns, &ComputerItemWatcher::itemUpdated, this, &ComputerModel::onItemUpdated);
}

void ComputerModel::onItemAdded(const ComputerItemData &data)
{
    int pos = findItem(data.url);
    if (pos > 0)   // update the item
        onItemUpdated(data.url);
    else {
        int i = 7;   // where the disk begin.
        for (; i < items.count(); i++)
            if (ComputerItemWatcher::typeCompare(data, items.at(i)))
                break;
        beginInsertRows(QModelIndex(), i, i);
        items.insert(i, data);
        endInsertRows();
    }
}

void ComputerModel::onItemRemoved(const QUrl &url)
{
    int pos = findItem(url);
    if (pos > 0) {
        beginRemoveRows(QModelIndex(), pos, pos);
        items.removeAt(pos);
        endRemoveRows();
    } else {
        qDebug() << "target item not found" << url;
    }
}

void ComputerModel::onItemUpdated(const QUrl &url)
{
    auto updateItemInfo = [this](int pos) {
        items.at(pos).info->refresh();
        view->update(this->index(pos, 0));
    };
    int pos = findItem(url);
    if (pos > 0) {
        updateItemInfo(pos);
    } else {
        pos = findItemByClearDeviceId(ComputerItemWatcher::getBlockDevIdByUrl(url));
        if (pos > 0) {
            updateItemInfo(pos);
            return;
        }
        qDebug() << "target item not found" << url;
    }
}

DPCOMPUTER_END_NAMESPACE
