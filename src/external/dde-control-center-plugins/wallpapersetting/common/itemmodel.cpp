// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemmodel.h"

using namespace dfm_wallpapersetting;

ItemModel::ItemModel(QObject *parent) : QAbstractItemModel(parent)
{

}

QModelIndex ItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= rowCount() || parent.isValid() || column != 0)
        return QModelIndex();

    return createIndex(row, 0);
}

QModelIndex ItemModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return items.size();
}

int ItemModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
    auto node = itemNode(index);
    QVariant ret;
    if (!node)
        return ret;

    switch (role) {
    case Qt::DisplayRole:
        ret = node->pixmap;
        break;
    case Qt::BackgroundRole:
        ret = node->color;
        break;
    default:
        break;
    }
    return ret;
}

Qt::ItemFlags ItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flg = 0;
    if (auto node = itemNode(index)) {
        flg |= Qt::ItemIsEnabled;
        if (node->selectable)
            flg |= Qt::ItemIsSelectable;
    }

    return flg;
}

void ItemModel::insertItem(int pos, ItemNodePtr it)
{
    if (pos < 0 || pos > rowCount() || it.isNull())
        pos = rowCount();

    beginInsertRows(QModelIndex(), pos, pos);
    items.insert(pos, it);
    endInsertRows();
}

void ItemModel::removeItem(const QString &item)
{
    auto idx = itemIndex(item);
    if (!idx.isValid())
        return;

    beginRemoveRows(QModelIndex(), idx.row(), idx.row());
    items.removeAt(idx.row());
    endRemoveRows();
}

ItemNodePtr ItemModel::itemNode(const QModelIndex &idx) const
{
    auto row = idx.row();
    if (row < 0 || row > rowCount())
        return {};

    return items.at(row);
}

QModelIndex ItemModel::itemIndex(const QString &item) const
{
    auto it = std::find_if(items.begin(), items.end(), [item](const ItemNodePtr &ptr) {
        return ptr->item == item;
    });

    if (it == items.end())
        return QModelIndex();
    auto row = items.indexOf(*it);
    return index(row, 0);
}

void ItemModel::resetData(const QList<ItemNodePtr> &list)
{
    beginResetModel();
    items = list;
    endResetModel();
}
