// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsidebarmodel.h"

#include "views/dfmsidebaritemdelegate.h"
#include "interfaces/dfmsidebaritem.h"

#include <QMimeData>
#include <QDebug>

#define MODELITEM_MIMETYPE "application/x-dfmsidebaritemmodeldata"

DFM_BEGIN_NAMESPACE

DFMSideBarModel::DFMSideBarModel(QObject *parent)
    : QStandardItemModel(parent)
{

}

bool DFMSideBarModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    // when drag onto the empty space of the area, just return false.
    if (column == -1 || row == -1 || !data)
        return false;

    Q_ASSERT(column == 0);

    auto isSeparator = [](DFMSideBarItem *item)->bool{
        return item && item->itemType()==DFMSideBarItem::Separator;
    };
    auto isItemDragEnabled = [](DFMSideBarItem *item)->bool{
        return item && item->flags().testFlag(Qt::ItemIsDragEnabled);
    };
    auto isTheSameGroup = [](DFMSideBarItem *item1, DFMSideBarItem *item2)->bool{
        return item1 && item2 && item1->groupName() == item2->groupName();
    };

    DFMSideBarItem * targetItem = this->itemFromIndex(row);
    DFMSideBarItem * sourceItem = nullptr;

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

        DFMSideBarItem * prevItem = itemFromIndex(row-1);
        // drag tag item to bottom, targetItem is null
        // drag bookmark item on the bookmark bottom separator, targetItem is Separator
        if ((!targetItem || isSeparator(targetItem)) && sourceItem != prevItem) {
            return isItemDragEnabled(prevItem) && isTheSameGroup(prevItem, sourceItem);
        }

        return false;
    }

    return QStandardItemModel::canDropMimeData(data, action, row, column, parent);
}

QMimeData *DFMSideBarModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData * data = QStandardItemModel::mimeData(indexes);
    if (!data)
        return nullptr;
    data->setData(MODELITEM_MIMETYPE, generateMimeData(indexes));
    return data;
}

QModelIndex DFMSideBarModel::indexFromItem(const DFMSideBarItem *item) const
{
    return QStandardItemModel::indexFromItem(item);
}

DFMSideBarItem *DFMSideBarModel::itemFromIndex(const QModelIndex &index) const
{
    QStandardItem *item = QStandardItemModel::itemFromIndex(index);
    DFMSideBarItem* castedItem = static_cast<DFMSideBarItem*>(item);

    return castedItem;
}

DFMSideBarItem *DFMSideBarModel::itemFromIndex(int index) const
{
    return itemFromIndex(this->index(index, 0));
}

QByteArray DFMSideBarModel::generateMimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return QByteArray();
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    stream << indexes.first().row();

    return encoded;
}

int DFMSideBarModel::getRowIndexFromMimeData(const QByteArray &data) const
{
    int row;
    QDataStream stream(data);
    stream >> row;

    return row;
}

DFM_END_NAMESPACE
