/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "views/sidebaritemdelegate.h"
#include "views/sidebaritem.h"

#include <QMimeData>
#include <QDebug>
#include <QtConcurrent>

#define MODELITEM_MIMETYPE "application/x-dfmsidebaritemmodeldata"

DPSIDEBAR_USE_NAMESPACE

namespace GlobalPrivate {
QByteArray generateMimeData(const QModelIndexList &indexes)
{
    if (indexes.isEmpty())
        return QByteArray();
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    stream << indexes.first().row();

    return encoded;
}

int getRowIndexFromMimeData(const QByteArray &data)
{
    int row;
    QDataStream stream(data);
    stream >> row;

    return row;
}
}   // namespace GlobalPrivate

/*!
 * \class SideBarModel
 * \brief
 */
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

    auto isSeparator = [](SideBarItem *item) -> bool {
        return item && dynamic_cast<DFMSideBarItemSeparator *>(item);
    };
    auto isItemDragEnabled = [](SideBarItem *item) -> bool {
        return item && item->flags().testFlag(Qt::ItemIsDragEnabled);
    };
    auto isTheSameGroup = [](SideBarItem *item1, SideBarItem *item2) -> bool {
        return item1 && item2 && item1->group() == item2->group();
    };

    SideBarItem *targetItem = this->itemFromIndex(row);
    SideBarItem *sourceItem = nullptr;

    // check if is item internal move by action and mimetype:
    if (action == Qt::MoveAction && data->formats().contains(MODELITEM_MIMETYPE)) {
        int oriRowIndex = GlobalPrivate::getRowIndexFromMimeData(data->data(MODELITEM_MIMETYPE));
        if (oriRowIndex >= 0) {
            sourceItem = this->itemFromIndex(oriRowIndex);
        }

        // normal drag tag or bookmark
        if (isItemDragEnabled(targetItem) && isTheSameGroup(sourceItem, targetItem)) {
            return true;
        }

        SideBarItem *prevItem = itemFromIndex(row - 1);
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
    QMimeData *data = QStandardItemModel::mimeData(indexes);
    if (!data)
        return nullptr;
    data->setData(MODELITEM_MIMETYPE, GlobalPrivate::generateMimeData(indexes));
    return data;
}

SideBarItem *SideBarModel::itemFromIndex(const QModelIndex &index) const
{
    QStandardItem *item = QStandardItemModel::itemFromIndex(index);
    SideBarItem *castedItem = static_cast<SideBarItem *>(item);

    return castedItem;
}

SideBarItem *SideBarModel::itemFromIndex(int index) const
{
    return itemFromIndex(this->index(index, 0));
}

bool SideBarModel::insertRow(int row, SideBarItem *item)
{
    if (!item)
        return false;

    if (0 > row)
        return false;

    if (rowCount() == 0) {
        QStandardItemModel::appendRow(item);
    } else {
        int endRowIndex = -1;
        int beginRowIndex = -1;
        // find insert group
        for (int rowIndex = rowCount() - 1; rowIndex >= 0; rowIndex--) {
            auto findedItem = dynamic_cast<SideBarItem *>(this->item(rowIndex, 0));

            if (!findedItem)
                return false;

            if (findedItem->group() == item->group()) {
                if (-1 == endRowIndex)
                    endRowIndex = rowIndex;
                else
                    beginRowIndex = rowIndex;
            }
        }

        if (-1 != endRowIndex && -1 != beginRowIndex) {
            int groupCount = endRowIndex - beginRowIndex + 1;
            // if the index is greater than the total number of elements in the group,
            // then inserted at the end of the group.
            if (row > groupCount)
                row = groupCount;
            QStandardItemModel::insertRow(row + beginRowIndex, item);
        } else {
            QStandardItemModel::appendRow(item);
        }
    }

    return true;
}

int SideBarModel::appendRow(SideBarItem *item)
{
    if (!item)
        return -1;

    QString currentGroup = item->group();
    auto controller = QtConcurrent::run([&]() {
        if (rowCount() == 0) {
            QStandardItemModel::appendRow(item);
            return 0;
        } else {
            //find insert group
            for (int row = rowCount() - 1; row >= 0; row--) {
                auto findedItem = dynamic_cast<SideBarItem *>(this->item(row, 0));

                if (!findedItem)
                    return -1;

                if (findedItem && findedItem->group() == currentGroup) {
                    QStandardItemModel::insertRow(row + 1, item);
                    return row + 1;
                }
            }
            QStandardItemModel::appendRow(item);
            return 0;
        }
    });
    controller.waitForFinished();

    return controller.result();
}

bool SideBarModel::removeRow(SideBarItem *item)
{
    if (!item)
        return false;

    auto controller = QtConcurrent::run([&]() {
        for (int row = rowCount() - 1; row >= 0; row--) {
            auto findedItem = dynamic_cast<SideBarItem *>(this->item(row, 0));
            if (item == findedItem) {
                QStandardItemModel::removeRow(row);
                return true;
            }
        }
        return false;
    });

    controller.waitForFinished();

    if (!controller.result())
        return false;
    else
        return true;
}

QStringList SideBarModel::groups() const
{
    QStringList list;
    auto controller = QtConcurrent::run([&]() {
        for (int row = rowCount() - 1; row <= 0; row--) {
            auto findedItem = dynamic_cast<SideBarItem *>(this->item(row, 0));
            if (!list.contains(findedItem->group()))
                list.push_front(findedItem->group());
        }
    });
    controller.waitForFinished();
    return list;
}
