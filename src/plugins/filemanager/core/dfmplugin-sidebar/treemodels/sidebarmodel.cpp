// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarmodel.h"
#include "sidebaritemdelegate.h"
#include "sidebaritem.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-framework/event/event.h>

#include <QMimeData>
#include <QDebug>
#include <QtConcurrent>

DPSIDEBAR_USE_NAMESPACE

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
        return item && dynamic_cast<SideBarItemSeparator *>(item);
    };
    auto isItemDragEnabled = [](SideBarItem *item) -> bool {
        return item && item->flags().testFlag(Qt::ItemIsDragEnabled);
    };
    auto isTheSameGroup = [](SideBarItem *item1, SideBarItem *item2) -> bool {
        return item1 && item2 && item1->group() == item2->group();
    };

    SideBarItem *targetItem = this->itemFromIndex(row, parent);

    if (isSeparator(targetItem))   //According to the requirement，sparator does not support to drop.
        return false;

    // check if is item internal move by action and mimetype:
    if (action == Qt::MoveAction) {
        SideBarItem *sourceItem = curDragItem;

        // normal drag tag or bookmark or quick access
        if (isItemDragEnabled(targetItem) && isTheSameGroup(sourceItem, targetItem))
            return true;

        SideBarItem *prevItem = itemFromIndex(row - 1, parent);
        // drag tag item to bottom, targetItem is null
        // drag bookmark item on the bookmark bottom separator, targetItem is Separator
        if ((!targetItem || isSeparator(targetItem)) && sourceItem != prevItem)
            return isItemDragEnabled(prevItem) && isTheSameGroup(prevItem, sourceItem);

        return false;
    }

    return QStandardItemModel::canDropMimeData(data, action, row, column, parent);
}

bool SideBarModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    return QStandardItemModel::dropMimeData(data, action, row, column, parent);
}

QMimeData *SideBarModel::mimeData(const QModelIndexList &indexes) const
{
    curDragItem = nullptr;
    QMimeData *data = QStandardItemModel::mimeData(indexes);
    if (!data)
        return nullptr;
    if (!indexes.isEmpty())
        curDragItem = itemFromIndex(indexes.first().row(), indexes.first().parent());
    return data;
}

SideBarItem *SideBarModel::itemFromIndex(const QModelIndex &index) const
{
    QStandardItem *item = QStandardItemModel::itemFromIndex(index);
    SideBarItem *castedItem = static_cast<SideBarItem *>(item);

    return castedItem;
}

SideBarItem *SideBarModel::itemFromIndex(int index, const QModelIndex &parent) const
{
    return itemFromIndex(this->index(index, 0, parent));
}

QList<SideBarItemSeparator *> SideBarModel::groupItems() const
{
    QList<SideBarItemSeparator *> items;

    for (int i = 0; i != rowCount(); ++i) {
        auto item = itemFromIndex(i);
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
        if (groupItem)
            items.append(groupItem);
    }

    return items;
}

QList<SideBarItem *> SideBarModel::subItems() const
{
    QList<SideBarItem *> items;
    QList<SideBarItemSeparator *> groups { groupItems() };

    for (auto groupItem : groups) {
        Q_ASSERT(groupItem);
        int childCount = groupItem->rowCount();
        for (int i = 0; i != childCount; ++i) {
            QStandardItem *childItem = groupItem->child(i);
            SideBarItem *subItem = static_cast<SideBarItem *>(childItem);
            if (subItem)
                items.append(subItem);
        }
    }

    return items;
}

QList<SideBarItem *> SideBarModel::subItems(const QString &groupName) const
{
    QList<SideBarItem *> items;
    QList<SideBarItemSeparator *> groups { groupItems() };

    for (auto groupItem : groups) {
        Q_ASSERT(groupItem);
        if (groupItem->group() != groupName)
            continue;
        int childCount = groupItem->rowCount();
        for (int i = 0; i != childCount; ++i) {
            QStandardItem *childItem = groupItem->child(i);
            SideBarItem *subItem = static_cast<SideBarItem *>(childItem);
            if (subItem)
                items.append(subItem);
        }
    }
    return items;
}

bool SideBarModel::insertRow(int row, SideBarItem *item)
{
    if (!item)
        return false;

    if (0 > row)
        return false;

    if (findRowByUrl(item->url()).row() > 0)
        return true;

    SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
    if (groupItem) {   //top item
        QStandardItemModel::insertRow(row + 1, item);   //insert the top item
        return true;
    } else {   //sub item
        int count = this->rowCount();
        for (int i = 0; i < count; i++) {
            const QModelIndex &index = this->index(i, 0);
            if (!index.isValid())
                continue;
            if (index.data(SideBarItem::Roles::kItemGroupRole).toString() != item->group())
                continue;
            SideBarItem *groupItem = this->itemFromIndex(index);
            if (groupItem) {
                int rows = groupItem->rowCount();
                if (row == 0 || (row > 0 && row < rows))
                    groupItem->insertRow(row, item);
                else if (row >= rows)
                    groupItem->appendRow(item);
                else if (row == -1)
                    groupItem->insertRow(0, item);
            }
            return true;
        }
    }

    return true;
}

int SideBarModel::appendRow(SideBarItem *item, bool direct)
{
    if (!item)
        return -1;

    auto r = findRowByUrl(item->url()).row();
    if (r > 0)
        return r;

    SideBarItemSeparator *topItem = dynamic_cast<SideBarItemSeparator *>(item);
    SideBarItem *groupOther = nullptr;
    if (topItem) {   //Top item
        auto t = topItem->group();
        QStandardItemModel::appendRow(item);
        return rowCount() - 1;   //The return value is the index of top item.
    } else {   //Sub item
        int count = this->rowCount();
        for (int i = 0; i < count; i++) {
            const QModelIndex &index = this->index(i, 0);
            if (!index.isValid())
                continue;
            QString groupId = index.data(SideBarItem::Roles::kItemGroupRole).toString();
            if (groupId == DefaultGroup::kOther)
                groupOther = this->itemFromIndex(i);
            if (groupId != item->group())
                continue;
            SideBarItem *groupItem = this->itemFromIndex(i);
            bool itemInserted = false;
            int row = 0;
            for (; !direct && row < groupItem->rowCount(); row++) {
                QStandardItem *childItem = groupItem->child(row);
                auto tmpItem = dynamic_cast<SideBarItem *>(childItem);
                if (!tmpItem)
                    continue;

                //Sort for devices group and network group, all so for quick access group.
                //Both of Computer plugin and bookmark plugin are following the the `hook_Group_Sort` event.
                bool sorted = { dpfHookSequence->run("dfmplugin_sidebar", "hook_Group_Sort", groupId, item->subGourp(), item->url(), tmpItem->url()) };
                if (sorted) {
                    groupItem->insertRow(row, item);
                    itemInserted = true;
                    break;
                }
            }
            if (!itemInserted)
                groupItem->appendRow(item);

            return row;   // The position after sorted
        }
    }
    if (groupOther && !topItem) {   //If can not find out the parent item, just append it to Group_Other
        groupOther->appendRow(item);
        fmInfo() << "Item added to groupOther";
        return groupOther->rowCount() - 1;
    }
    QStandardItemModel::appendRow(item);
    fmInfo() << "Item added to the end of sidebar.";
    return rowCount() - 1;
}

bool SideBarModel::removeRow(const QUrl &url)
{
    if (!url.isValid())
        return false;

    int count = this->rowCount();
    for (int i = 0; i < count; i++) {
        const QModelIndex &index = this->index(i, 0);   //top item index
        if (index.isValid()) {
            QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
            SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
            if (!groupItem)
                continue;
            int childCount = groupItem->rowCount();
            for (int j = 0; j < childCount; j++) {
                QStandardItem *childItem = groupItem->child(j);
                SideBarItem *subItem = static_cast<SideBarItem *>(childItem);
                if (!subItem)
                    continue;
                if (DFMBASE_NAMESPACE::UniversalUtils::urlEquals(subItem->url(), url)) {
                    QStandardItemModel::removeRows(j, 1, groupItem->index());
                    return true;
                }
            }
        }
    }

    return false;
}

void SideBarModel::updateRow(const QUrl &url, const ItemInfo &newInfo)
{
    if (!url.isValid())
        return;
    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);   //Top item
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
        if (!groupItem)
            continue;
        int childCount = groupItem->rowCount();
        for (int j = 0; j < childCount; j++) {
            QStandardItem *childItem = groupItem->child(j);
            SideBarItem *subItem = static_cast<SideBarItem *>(childItem);
            if (!subItem)
                continue;
            bool foundByCb = subItem->itemInfo().findMeCb && subItem->itemInfo().findMeCb(subItem->url(), url);

            if (foundByCb || DFMBASE_NAMESPACE::UniversalUtils::urlEquals(subItem->url(), url)) {
                subItem->setIcon(newInfo.icon);
                subItem->setText(newInfo.displayName);
                subItem->setUrl(newInfo.url);
                subItem->setFlags(newInfo.flags);
                subItem->setGroup(newInfo.group);
                Qt::ItemFlags flags = subItem->flags();
                if (newInfo.isEditable)
                    flags |= Qt::ItemIsEditable;
                else
                    flags &= (~Qt::ItemIsEditable);
                subItem->setFlags(flags);
                return;
            }
        }
    }
}

QModelIndex SideBarModel::findRowByUrl(const QUrl &url) const
{
    QModelIndex retIndex;

    int count = this->rowCount();   //The top row count
    for (int i = 0; i < count; i++) {
        const QModelIndex &index = this->index(i, 0);   //top item index
        if (index.isValid()) {
            QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
            SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
            if (!groupItem)
                continue;
            int childCount = groupItem->rowCount();
            for (int j = 0; j < childCount; j++) {
                QStandardItem *childItem = groupItem->child(j);
                SideBarItem *subItem = static_cast<SideBarItem *>(childItem);
                if (!subItem)
                    continue;
                if (DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, subItem->url()))
                    return subItem->index();
            }
        }
    }

    return retIndex;
}

void SideBarModel::addEmptyItem()
{
    //Attention!
    //The current sidebar does not support external plugins to add groups.
    //If this feature is implemented in the future, it is necessary to move the emptyItem item appropriately
    int count = rowCount();
    QSize emptyItemsize = QSize(10, 10);
    if (count > 0) {
        QStandardItem *lastItem = item(count - 1);
        if (lastItem && lastItem->sizeHint() == emptyItemsize)
            return;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    auto emptyItem = new QStandardItem("");
    emptyItem->setFlags(Qt::NoItemFlags);
    emptyItem->setSizeHint(emptyItemsize);

    QStandardItemModel::appendRow(emptyItem);
    endInsertRows();
}
