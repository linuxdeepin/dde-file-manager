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
    if (column == -1 || row == -1) return false;

    Q_ASSERT(column == 0);

    DFMSideBarItem * targetItem = this->itemFromIndex(row);
    DFMSideBarItem * sourceItem = nullptr;

    // check if is item internal move by action and mimetype:
    if (action == Qt::MoveAction && data->formats().contains(MODELITEM_MIMETYPE)) {
        if (targetItem && targetItem->flags().testFlag(Qt::ItemIsDragEnabled)) {
            int oriRowIndex = getRowIndexFromMimeData(data->data(MODELITEM_MIMETYPE));
            if (oriRowIndex >= 0) {
                sourceItem = this->itemFromIndex(oriRowIndex);
            }
            if (sourceItem) {
                // known issue: we cannot drag the item to the bottom of the group.
                if (sourceItem->groupName() != targetItem->groupName()) {
                    return false;
                }
            }
        } else {
            return false;
        }
    }

    return QStandardItemModel::canDropMimeData(data, action, row, column, parent);
}

QMimeData *DFMSideBarModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData * data = QStandardItemModel::mimeData(indexes);

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
