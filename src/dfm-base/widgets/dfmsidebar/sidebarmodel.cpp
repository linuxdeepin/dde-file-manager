/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "sidebarmodel.h"
#include "sidebaritemdelegate.h"
#include "sidebaritem.h"

#include <QMimeData>
#include <QDebug>

#define MODELITEM_MIMETYPE "application/x-dfmsidebaritemmodeldata"
DFMBASE_BEGIN_NAMESPACE
SideBarModel::SideBarModel(QObject *parent)
    : QStandardItemModel(parent)
{

}

bool SideBarModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    // when drag onto the empty space of the area, just return false.
    if (column == -1 || row == -1 || !data)
        return false;

    Q_ASSERT(column == 0);

    auto isSeparator = [](SideBarItem *item)->bool{
        return item && dynamic_cast<DFMSideBarItemSeparator*>(item);
    };
    auto isItemDragEnabled = [](SideBarItem *item)->bool{
        return item && item->flags().testFlag(Qt::ItemIsDragEnabled);
    };
    auto isTheSameGroup = [](SideBarItem *item1, SideBarItem *item2)->bool{
        return item1 && item2 && item1->group() == item2->group();
    };

    SideBarItem * targetItem = this->itemFromIndex(row);
    SideBarItem * sourceItem = nullptr;

    // check if is item internal move by action and mimetype:
    if (action == Qt::MoveAction && data->formats().contains(MODELITEM_MIMETYPE)) {
        int oriRowIndex = getRowIndexFromMimeData(data->data(MODELITEM_MIMETYPE));
        if (oriRowIndex >= 0) {
            sourceItem = this->itemFromIndex(oriRowIndex);
        }

        // normal drag tag or bookmark
        if (isItemDragEnabled(targetItem) && isTheSameGroup(sourceItem, targetItem)) {
            return true;
        }

        SideBarItem * prevItem = itemFromIndex(row-1);
        // drag tag item to bottom, targetItem is null
        // drag bookmark item on the bookmark bottom separator, targetItem is Separator
        if ((!targetItem || isSeparator(targetItem)) && sourceItem != prevItem) {
            return isItemDragEnabled(prevItem) && isTheSameGroup(prevItem, sourceItem);
        }

        return false;
    }

    return QStandardItemModel::canDropMimeData(data, action, row, column, parent);
}

QMimeData *SideBarModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData * data = QStandardItemModel::mimeData(indexes);
    if (!data)
        return nullptr;
    data->setData(MODELITEM_MIMETYPE, generateMimeData(indexes));
    return data;
}

QModelIndex SideBarModel::indexFromItem(const SideBarItem *item) const
{
    return QStandardItemModel::indexFromItem(item);
}

SideBarItem *SideBarModel::itemFromIndex(const QModelIndex &index) const
{
    QStandardItem *item = QStandardItemModel::itemFromIndex(index);
    SideBarItem* castedItem = static_cast<SideBarItem*>(item);

    return castedItem;
}

SideBarItem *SideBarModel::itemFromIndex(int index) const
{
    return itemFromIndex(this->index(index, 0));
}

QByteArray SideBarModel::generateMimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return QByteArray();
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    stream << indexes.first().row();

    return encoded;
}

int SideBarModel::getRowIndexFromMimeData(const QByteArray &data) const
{
    int row;
    QDataStream stream(data);
    stream >> row;

    return row;
}

DFMBASE_END_NAMESPACE
