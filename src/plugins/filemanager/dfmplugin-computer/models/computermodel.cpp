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
#include "file/entry/entryfileinfo.h"
#include "utils/fileutils.h"

#include <QVector>
DPCOMPUTER_BEGIN_NAMESPACE

ComputerModel::ComputerModel(QObject *parent)
    : QAbstractItemModel(parent),
      watcher(new ComputerItemWatcher(this))
{
    view = qobject_cast<ComputerView *>(parent);
    items = watcher->items();

    initConn();
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

void ComputerModel::initConn()
{
    connect(watcher.data(), &ComputerItemWatcher::itemAdded, this, [this](const ComputerItemData &data) {
        int pos = findItem(data.url);
        if (pos > 0) {
            // update item
            this->beginRemoveRows(QModelIndex(), pos, pos);
            items.at(pos).info->refresh();
            this->endRemoveRows();
        } else {
            int i = 7;   // the Disks begin
            for (; i < items.count(); i++) {
                if (ComputerItemWatcher::typeCompare(data, items.at(i))) {
                    this->beginInsertRows(QModelIndex(), i, i);
                    this->items.insert(i, data);
                    this->endInsertRows();
                    return;
                }
            }
            this->beginInsertRows(QModelIndex(), i, i);
            this->items.append(data);
            this->endInsertRows();
        }
    });
    connect(watcher.data(), &ComputerItemWatcher::itemRemoved, this, [this](const QUrl &url) {
        int pos = findItem(url);
        if (pos > 0) {
            this->beginRemoveRows(QModelIndex(), pos, pos);
            items.removeAt(pos);
            this->endRemoveRows();
        }
    });
    connect(watcher.data(), &ComputerItemWatcher::itemUpdated, this, [this](const QUrl &url) {
        int pos = findItem(url);
        if (pos > 0) {
            items.at(pos).info->refresh();
            this->view->update(this->index(pos, 0));
        }
    });
}

DPCOMPUTER_END_NAMESPACE
