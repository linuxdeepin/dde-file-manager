/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "private/sidebarmodel_p.h"
#include "sidebarmodel.h"
#include "sidebaritemdelegate.h"
#include "sidebaritem.h"

#include <QMimeData>
#include <QDebug>
#include <QtConcurrent>

#define MODELITEM_MIMETYPE "application/x-dfmsidebaritemmodeldata"
DFMBASE_BEGIN_NAMESPACE

namespace GlobalPrivate
{
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
} //namespace GlobalPrivate

SideBarModelPrivate::SideBarModelPrivate(SideBarModel *qq)
    : QObject(qq)
    , q_ptr(qq)
{

}

SideBarModel::SideBarModel(QObject *parent)
    : QStandardItemModel(parent)
    , d(new SideBarModelPrivate(this))
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
        int oriRowIndex = GlobalPrivate::getRowIndexFromMimeData(data->data(MODELITEM_MIMETYPE));
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
    data->setData(MODELITEM_MIMETYPE, GlobalPrivate::generateMimeData(indexes));
    return data;
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

bool SideBarModel::insertRow(int row, SideBarItem *item)
{
    if (!item)
        return false;

    if (0 > row)
        return false;

    QString currGroup = item->group();

    int groupCount = -1;
    int endRowIndex = -1;
    int beginRowIndex = -1;

    auto controller = QtConcurrent::run([&](){
        if (rowCount() == 0) {
            QStandardItemModel::appendRow(item);
            return true;
        } else {
            //find insert group
            for (int row = rowCount() - 1; row >= 0; row -- ) {
                auto findedItem = dynamic_cast<SideBarItem*>(this->item(row, 0));

                if (!findedItem)
                    return false;

                if (findedItem && endRowIndex == -1
                        && findedItem->group() == currGroup) {
                    endRowIndex = row;
                }

                if (findedItem && endRowIndex != -1
                        && findedItem->group() == currGroup) {
                    beginRowIndex = row;
                }
            }

            if (-1 != endRowIndex && -1 != beginRowIndex) {
                groupCount = endRowIndex - beginRowIndex;
            } else {
                QStandardItemModel::appendRow(item);
                return true;
            }
        }
        return true;
    });
    controller.waitForFinished();

    if (!controller.result())
        return false;

    if(row >= groupCount)
        return false;

    QStandardItemModel::insertRow(row + beginRowIndex, item);
    return true;
}

int SideBarModel::appendRow(SideBarItem *item)
{
    if (!item)
        return -1;

    QString currentGroup = item->group();
    auto controller = QtConcurrent::run([&](){
        if (rowCount() == 0) {
            QStandardItemModel::appendRow(item);
            return 0;
        } else {
            //find insert group
            for (int row = rowCount() - 1; row >= 0; row -- ) {
                auto findedItem = dynamic_cast<SideBarItem*>(this->item(row, 0));

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

    auto controller = QtConcurrent::run([&](){
        for (int row = rowCount() - 1; row >= 0; row -- ) {
            auto findedItem = dynamic_cast<SideBarItem*>(this->item(row, 0));
            if (item == findedItem) {
                QStandardItemModel::removeRow(row);
                return true;
            }
        }
        return false;
    });

    controller.waitForFinished();

    if(!controller.result())
        return false;
    else
        return true;
}

QStringList SideBarModel::groups() const
{
    QStringList list;
    auto controller = QtConcurrent::run([&](){
        for (int row = rowCount() - 1; row <= 0; row-- ) {
            auto findedItem = dynamic_cast<SideBarItem*>(this->item(row, 0));
            if (!list.contains(findedItem->group()))
                list.push_front(findedItem->group());
        }
    });
    controller.waitForFinished();
    return list;
}

DFMBASE_END_NAMESPACE
