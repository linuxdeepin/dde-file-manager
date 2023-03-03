// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarmodel.h"
#include "views/sidebaritemdelegate.h"
#include "views/sidebaritem.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"

#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/event/event.h>

#include <QMimeData>
#include <QDebug>
#include <QtConcurrent>

static constexpr char kModelitemMimetype[] { "application/x-dfmsidebaritemmodeldata" };

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
    QMutexLocker lk(&locker);
    if (!item)
        return false;

    if (0 > row)
        return false;

    if (findRowByUrl(item->url()) > 0)
        return true;

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
    QMutexLocker lk(&locker);

    if (!item)
        return -1;

    auto r = findRowByUrl(item->url());
    if (r > 0)
        return r;

    if (rowCount() == 0) {
        QStandardItemModel::appendRow(item);
        return 0;
    } else {
        const QString &currentGroup = item->group();
        const QString &subGroup = item->subGourp();

        bool foundGroup = false;
        for (int r = 0; r < rowCount(); r++) {
            auto tmpItem = dynamic_cast<SideBarItem *>(this->item(r, 0));
            if (!tmpItem)
                continue;
            if (tmpItem->group() == currentGroup) {
                foundGroup = true;
                bool sorted { dpfHookSequence->run("dfmplugin_sidebar", "hook_Group_Sort",
                                                   currentGroup, subGroup, item->url(), tmpItem->url()) };
                if (sorted) {
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
    QMutexLocker lk(&locker);

    if (!item)
        return false;

    for (int row = rowCount() - 1; row >= 0; row--) {
        auto foundItem = dynamic_cast<SideBarItem *>(this->item(row, 0));
        if (item == foundItem) {
            QStandardItemModel::removeRow(row);
            return true;
        }
    }

    return false;
}

bool SideBarModel::removeRow(const QUrl &url)
{
    QMutexLocker lk(&locker);

    if (!url.isValid())
        return false;

    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);
        if (item && DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, item->url())) {
            QStandardItemModel::removeRow(r);
            return true;
        }
    }
    return false;
}

void SideBarModel::updateRow(const QUrl &url, const ItemInfo &newInfo)
{
    QMutexLocker lk(&locker);

    if (!url.isValid())
        return;

    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);
        if (item && item->url() == url) {
            item->setIcon(newInfo.icon);
            item->setText(newInfo.displayName);
            item->setUrl(newInfo.url);
            item->setFlags(newInfo.flags);
            item->setGroup(newInfo.group);
            Qt::ItemFlags flags = item->flags();
            if (newInfo.isEditable)
                flags |= Qt::ItemIsEditable;
            else
                flags &= (~Qt::ItemIsEditable);
            item->setFlags(flags);
        }
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
