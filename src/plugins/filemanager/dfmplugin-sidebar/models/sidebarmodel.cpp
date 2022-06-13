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
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"

#include "dfm-base/utils/universalutils.h"

#include <QMimeData>
#include <QDebug>
#include <QtConcurrent>

static constexpr char kModelitemMimetype[] { "application/x-dfmsidebaritemmodeldata" };

DSB_FM_USE_NAMESPACE
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
        return item && dynamic_cast<SideBarItemSeparator *>(item);
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
    if (action == Qt::MoveAction && data->formats().contains(kModelitemMimetype)) {
        int oriRowIndex = GlobalPrivate::getRowIndexFromMimeData(data->data(kModelitemMimetype));
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
    data->setData(kModelitemMimetype, GlobalPrivate::generateMimeData(indexes));
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
        int groupStart = -1;
        int groupEnd = -1;
        // find insert group
        for (int r = 0; r < rowCount(); r++) {
            auto foundItem = dynamic_cast<SideBarItem *>(this->item(r, 0));
            if (!foundItem)
                continue;

            if (foundItem->group() == item->group()) {
                if (-1 == groupStart)
                    groupStart = r;
                groupEnd = r;
            } else {
                if (groupStart != -1)
                    break;
            }
        }

        if (-1 != groupEnd && -1 != groupStart) {
            int groupCount = groupEnd - groupStart;   // don't treat the splitter as a group item
            // if the index is greater than the total number of elements in the group,
            // then inserted at the end of the group.
            if (row > groupCount)
                row = groupCount;
            // all item must be insterted below group splitter, so:
            row += 1;
            QStandardItemModel::insertRow(row + groupStart, item);
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

    if (rowCount() == 0) {
        QStandardItemModel::appendRow(item);
        return 0;
    } else {
        const QString &currentGroup = item->group();
        const QString &subGroup = item->subGourp();
        auto sortFunc = SideBarHelper::sortFunc(subGroup);

        bool foundGroup = false;
        for (int r = 0; r < rowCount(); r++) {
            auto tmpItem = dynamic_cast<SideBarItem *>(this->item(r, 0));
            if (!tmpItem)
                continue;
            if (tmpItem->group() == currentGroup) {
                foundGroup = true;
                if (sortFunc && sortFunc(item->url(), tmpItem->url())) {
                    QStandardItemModel::insertRow(r, item);
                    return r;
                }
            } else {
                if (foundGroup) {   // if already found group, then insert the item after the last same group item's position
                    QStandardItemModel::insertRow(r, item);
                    return r;
                }
            }
        }
    }

    QStandardItemModel::appendRow(item);
    return rowCount() - 1;
}

bool SideBarModel::removeRow(SideBarItem *item)
{
    if (!item)
        return false;

    for (int row = rowCount() - 1; row >= 0; row--) {
        auto foundItem = dynamic_cast<SideBarItem *>(this->item(row, 0));
        if (item == foundItem) {
            SideBarInfoCacheMananger::instance()->removeBindedItemInfo(item->url());
            QStandardItemModel::removeRow(row);
            return true;
        }
    }

    return false;
}

bool SideBarModel::removeRow(const QUrl &url)
{
    if (!url.isValid())
        return false;

    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);
        QUrl itemUrl { item->url() };
        if (item && DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, itemUrl)) {
            SideBarInfoCacheMananger::instance()->removeBindedItemInfo(item->url());
            QStandardItemModel::removeRow(r);
            return true;
        }
    }
    return false;
}

void SideBarModel::updateRow(const QUrl &url, const SideBar::ItemInfo &newInfo)
{
    if (!url.isValid())
        return;

    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);
        if (item->url() == url) {
            item->setItemInfo(newInfo);
            item->setIcon(QIcon::fromTheme(newInfo.iconName));
            item->setText(newInfo.text);
            item->setUrl(newInfo.url);
            item->setFlags(newInfo.flags);
        }
    }
}

void SideBarModel::updateRow(const QUrl &url, const QString &newName, bool editable)
{
    if (!url.isValid())
        return;

    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);
        if (item->url() == url) {
            item->setText(newName);
            Qt::ItemFlags flags = item->flags();
            if (editable)
                flags |= Qt::ItemIsEditable;
            else
                flags &= (~Qt::ItemIsEditable);
            item->setFlags(flags);
        }
    }
}

void SideBarModel::updateRow(const QUrl &url, const QIcon &newIcon)
{
    if (!url.isValid())
        return;

    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);
        if (item->url() == url)
            item->setIcon(newIcon);
    }
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

int SideBarModel::findRowByUrl(const QUrl &url)
{
    int ret { -1 };
    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);
        if (DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, item->url())) {
            ret = r;
            break;
        }
    }
    return ret;
}

int SideBarModel::findLastPosOf(const QString &group)
{
    for (int r = rowCount() - 1; r >= 0; r--) {
        auto item = itemFromIndex(index(r, 0));
        if (item && item->group() == group)
            return r;
    }
    return -1;
}

void SideBarModel::sortGroup(const QString &group, const QList<QUrl> &order)
{
    int r = findLastPosOf(group);
    if (r < 0)
        return;

    beginResetModel();
    int firstGroup = -1;
    QMap<QUrl, SideBarItem *> items;
    for (; r > 0; r--) {   // the index 0 is a group
        auto item = itemFromIndex(index(r, 0));
        if (item && item->group() == group && item->url().isValid()) {
            items.insert(item->url(), new SideBarItem(*item));
            QStandardItemModel::removeRow(r);   // takeItem do not reduce the rowCount() of model, might be a bug.
            firstGroup = r;
        }
    }

    for (auto url : order)
        insertRow(firstGroup++, items.value(url));
    endResetModel();
}
