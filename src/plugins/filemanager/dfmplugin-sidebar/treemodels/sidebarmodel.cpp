// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarmodel.h"
#include "treeviews/sidebaritem.h"
#include "treeviews/sidebarwidget.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarfilewatcher.h"
#include "utils/sidebarinfocachemananger.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/filenamesorter.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-framework/event/event.h>

#include <QMimeData>
#include <QDebug>
#include <QtConcurrent>
#include <QStack>
#include <QTreeView>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

/*!
 * \class SideBarModel
 * \brief
 */
SideBarModel::SideBarModel(QObject *parent)
    : QStandardItemModel(parent)
{
    fileWatcher = new SidebarFileWatcher(this);
    connect(fileWatcher, &SidebarFileWatcher::directoryCreated, this, &SideBarModel::onDirectoryCreated);
    connect(fileWatcher, &SidebarFileWatcher::directoryRemoved, this, &SideBarModel::onDirectoryRemoved);
    connect(fileWatcher, &SidebarFileWatcher::directoryRenamed, this, &SideBarModel::onDirectoryRenamed);
}

bool SideBarModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    // when drag onto the empty space of the area, just return false.
    if (column == -1 || row == -1 || !data) {
        fmDebug() << "Drop rejected: invalid parameters, column:" << column << "row:" << row << "data:" << (data != nullptr);
        return false;
    }

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

    auto isSourceItemValid = [this](SideBarItem *item) -> bool {
        for (const auto &group : groupItems()) {
            for (int row = 0; row < group->rowCount(); ++row) {
                if (group->child(row) == item) {
                    return true;
                }
            }
        }
        return false;
    };
    SideBarItem *targetItem = this->itemFromIndex(row, parent);

    if (isSeparator(targetItem)) {   // According to the requirement，sparator does not support to drop.
        fmDebug() << "Drop rejected: target is separator";
        return false;
    }

    // check if is item internal move by action and mimetype:
    if (action == Qt::MoveAction) {
        SideBarItem *sourceItem = curDragItem;

        if (!isSourceItemValid(sourceItem)) {
            fmWarning() << "Drop rejected: invalid source item";
            return false;
        }

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
    if (!canDropMimeData(data, action, row, column, parent)) {
        fmWarning() << "Drop operation failed: canDropMimeData check failed";
        return false;
    }

    return QStandardItemModel::dropMimeData(data, action, row, column, parent);
}

QMimeData *SideBarModel::mimeData(const QModelIndexList &indexes) const
{
    curDragItem = nullptr;
    QMimeData *data = QStandardItemModel::mimeData(indexes);
    if (!data) {
        fmWarning() << "Failed to create mime data";
        return nullptr;
    }
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
    if (!item) {
        fmWarning() << "Insert row failed: item is null";
        return false;
    }

    if (0 > row) {
        fmWarning() << "Insert row failed: invalid row index:" << row;
        return false;
    }

    if (findRowByUrl(item->url()).row() > 0)
        return true;

    SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
    if (groupItem) {   // top item
        QStandardItemModel::insertRow(row + 1, item);   // insert the top item
        setItemAccessibleName(item);
        return true;
    } else {   // sub item
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
                if (row == 0 || (row > 0 && row < rows)) {
                    groupItem->insertRow(row, item);
                    setItemAccessibleName(item);
                } else if (row >= rows) {
                    groupItem->appendRow(item);
                    setItemAccessibleName(item);
                } else if (row == -1) {
                    groupItem->insertRow(0, item);
                    setItemAccessibleName(item);
                }
            }
            return true;
        }
    }

    return true;
}

int SideBarModel::appendRow(SideBarItem *item, bool direct)
{
    if (!item) {
        fmWarning() << "Append row failed: item is null";
        return -1;
    }

    auto r = findRowByUrl(item->url()).row();
    if (r > 0)
        return r;

    SideBarItemSeparator *topItem = dynamic_cast<SideBarItemSeparator *>(item);
    SideBarItem *groupOther = nullptr;
    if (topItem) {   // Top item
        auto t = topItem->group();
        QStandardItemModel::appendRow(item);
        setItemAccessibleName(item);
        return rowCount() - 1;   // The return value is the index of top item.
    } else {   // Sub item
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

                // Sort for devices group and network group, all so for quick access group.
                // Both of Computer plugin and bookmark plugin are following the the `hook_Group_Sort` event.
                bool sorted = { dpfHookSequence->run("dfmplugin_sidebar", "hook_Group_Sort", groupId, item->subGourp(), item->url(), tmpItem->url()) };
                if (sorted) {
                    groupItem->insertRow(row, item);
                    setItemAccessibleName(item);
                    itemInserted = true;
                    break;
                }
            }
            if (!itemInserted) {
                groupItem->appendRow(item);
                setItemAccessibleName(item);
            }

            return row;   // The position after sorted
        }
    }
    if (groupOther && !topItem) {   // If can not find out the parent item, just append it to Group_Other
        groupOther->appendRow(item);
        setItemAccessibleName(item);
        fmInfo() << "Item added to groupOther";
        return groupOther->rowCount() - 1;
    }
    QStandardItemModel::appendRow(item);
    setItemAccessibleName(item);
    fmInfo() << "Item added to the end of sidebar.";
    return rowCount() - 1;
}

bool SideBarModel::removeRow(const QUrl &url)
{
    if (!url.isValid()) {
        fmWarning() << "Remove row failed: invalid URL:" << url;
        return false;
    }

    int count = this->rowCount();
    for (int i = 0; i < count; i++) {
        const QModelIndex &index = this->index(i, 0);   // top item index
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

    fmWarning() << "Item not found for removal, URL:" << url;
    return false;
}

void SideBarModel::updateRow(const QUrl &url, const ItemInfo &newInfo)
{
    if (!url.isValid()) {
        fmWarning() << "Update row failed: invalid URL:" << url;
        return;
    }

    for (int r = 0; r < rowCount(); r++) {
        auto item = itemFromIndex(r);   // Top item
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
                setItemAccessibleName(subItem);
                return;
            }
        }
    }

    fmWarning() << "Item not found for update, URL:" << url;
}

QModelIndex SideBarModel::findRowByUrl(const QUrl &url) const
{
    QModelIndex retIndex;

    int count = this->rowCount();   // The top row count
    for (int i = 0; i < count; i++) {
        const QModelIndex &index = this->index(i, 0);   // top item index
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

    fmDebug() << "Row not found for URL:" << url;
    return retIndex;
}

QModelIndex SideBarModel::findGroupIndex(const QString &name) const
{
    for (int i = 0; i < rowCount(); ++i) {
        auto item = itemFromIndex(i);
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(item);
        if (groupItem && groupItem->group() == name)
            return index(i, 0);
    }
    fmWarning() << "Group not found in sidebar!" << name;
    return {};
}

QModelIndexList SideBarModel::findRowsByUrlRecursive(const QUrl &url, const QModelIndex &parent) const
{
    QModelIndexList ret;
    QStack<QModelIndex> stack;
    stack.push(parent);

    while (!stack.isEmpty()) {
        auto current = stack.pop();
        for (int i = 0; i < rowCount(current); ++i) {
            auto idx = index(i, 0, current);
            if (!idx.isValid())
                continue;

            auto item = itemFromIndex(idx);
            if (!item)
                continue;

            if (UniversalUtils::urlEquals(url, item->url())
                || UniversalUtils::urlEquals(url, item->targetUrl())
                || (item->itemInfo().findMeCb && item->itemInfo().findMeCb(item->url(), url))) {
                ret << idx;
                continue;
            }

            if (UniversalUtils::isParentUrl(url, item->url())
                || UniversalUtils::isParentUrl(url, item->targetUrl())) {
                stack.push(idx);
            }
        }
    }
    return ret;
}

void SideBarModel::addEmptyItem()
{
    // Attention!
    // The current sidebar does not support external plugins to add groups.
    // If this feature is implemented in the future, it is necessary to move the emptyItem item appropriately
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

void SideBarModel::onItemExpanded(const QModelIndex &index)
{
    SideBarItem *item = itemFromIndex(index);
    if (!item)
        return;

    QUrl url = item->targetUrl();
    if (url.isEmpty())
        url = item->url();

    m_urlIndexMap.insert(url, QPersistentModelIndex(index));

    if (fileWatcher)
        fileWatcher->watchDirectory(url);
}

void SideBarModel::onItemCollapsed(const QModelIndex &index)
{
    SideBarItem *item = itemFromIndex(index);
    if (!item)
        return;

    QUrl url = item->targetUrl();
    if (url.isEmpty())
        url = item->url();

    // Before unwatching, make sure no other sidebar view still has this url expanded.
    auto isExpandedInAnyView = [](const QModelIndex &idx) {
        if (!idx.isValid())
            return false;

        auto sbs = SideBarHelper::allSideBar();
        return std::any_of(sbs.cbegin(), sbs.cend(), [idx](SideBarWidget *w) {
            auto v = qobject_cast<QTreeView *>(w->view());
            return v && v->isExpanded(idx);
        });
    };
    auto partGrp = findGroupIndex(DefaultGroup::kDevice);
    if (!partGrp.isValid())
        return;
    auto idxes = findRowsByUrlRecursive(url, partGrp);
    bool needWatching = std::any_of(idxes.cbegin(), idxes.cend(), isExpandedInAnyView);

    if (!needWatching)
        m_urlIndexMap.remove(url);

    if (fileWatcher && !needWatching) {
        fmDebug() << url << "in sidebar no need to be watched anymore.";
        fileWatcher->unwatchDirectory(url);
    }
}

void SideBarModel::addSubItems(const QModelIndex &index, const QList<QUrl> &urls)
{
    SideBarItem *parentItem = itemFromIndex(index);
    if (!parentItem) {
        fmWarning() << "cannot find parent sidebar item!" << index;
        return;
    }

    QSet<QUrl> urlSet(urls.begin(), urls.end());

    QSet<QUrl> existingSet;
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        SideBarItem *child = static_cast<SideBarItem *>(parentItem->child(i));
        if (child)
            existingSet.insert(child->url());
    }

    for (int i = parentItem->rowCount() - 1; i >= 0; --i) {
        SideBarItem *child = static_cast<SideBarItem *>(parentItem->child(i));
        if (child && !urlSet.contains(child->url())) {
            SideBarInfoCacheMananger::instance()->removeItemInfoCache(child->url());
            parentItem->removeRow(i);
        }
    }

    // Preserve the order of the incoming list (already sorted by
    // TraversalDirThread via FileNameSorter) rather than iterating over a
    // QSet (which discards order). addSubItem inserts each new item at the
    // correct position relative to existing children; since the incoming list
    // is already sorted, the linear scan in addSubItem hits the best case
    // (append at end) for each item, giving overall O(n) on the batch path.
    for (const QUrl &url : urls) {
        if (!existingSet.contains(url))
            addSubItem(index, url);
    }
}

void SideBarModel::onDirectoryCreated(const QUrl &parentUrl, const QUrl &url)
{
    auto it = m_urlIndexMap.find(parentUrl);
    if (it != m_urlIndexMap.end() && it->isValid()) {
        addSubItem(*it, url);
        return;
    }
    auto partGrp = findGroupIndex(DefaultGroup::kDevice);
    auto idxes = findRowsByUrlRecursive(parentUrl, partGrp);
    for (auto idx : idxes)
        addSubItem(idx, url);
}

void SideBarModel::onDirectoryRemoved(const QUrl &parentUrl, const QUrl &url)
{
    auto it = m_urlIndexMap.find(url);
    if (it != m_urlIndexMap.end() && it->isValid()) {
        removeSubItem(*it, url);
        return;
    }
    auto partGrp = findGroupIndex(DefaultGroup::kDevice);
    auto idxes = findRowsByUrlRecursive(url, partGrp);
    for (auto idx : idxes)
        removeSubItem(idx, url);
}

void SideBarModel::onDirectoryRenamed(const QUrl &parentUrl, const QUrl &oldUrl, const QUrl &newUrl)
{
    onDirectoryRemoved(parentUrl, oldUrl);
    onDirectoryCreated(parentUrl, newUrl);
}

void SideBarModel::addSubItem(const QModelIndex &index, const QUrl &url)
{
    auto info = InfoFactory::create<FileInfo>(url, dfmbase::Global::kCreateFileInfoSync);
    if (!info) {
        fmWarning() << "Failed to create FileInfo instance!" << url;
        return;
    }

    if (info->isAttributes(FileInfo::FileIsType::kIsHidden)
        && !Application::instance()->genericAttribute(dfmbase::Application::kShowedHiddenFiles).toBool()) {
        fmInfo() << "Hidden file created and not hidden files should not be displayed" << url;
        return;
    }

    // Hide system-default hidden directories (e.g. /<mount-point>/root, /<mount-point>/lost+found)
    // to keep the sidebar sub-tree consistent with the file view, which filters them via
    // FileSortWorker::isDefaultHiddenFile.
    if (!Application::instance()->genericAttribute(dfmbase::Application::kShowedHiddenFiles).toBool()
        && FileUtils::isDefaultHiddenFile(url)) {
        fmInfo() << "Default hidden directory should not be displayed in sidebar" << url;
        return;
    }

    SideBarItem *parentItem = itemFromIndex(index);
    if (!parentItem) {
        fmDebug() << "Failed to get parent item from index";
        return;
    }

    // Skip if the child already exists.
    int childCount = parentItem->rowCount();
    for (int i = 0; i < childCount; ++i) {
        SideBarItem *childItem = dynamic_cast<SideBarItem *>(parentItem->child(i));
        if (childItem && DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, childItem->url()))
            return;
    }

    QString group = parentItem->group();
    // Use the file-view display name (translated via SystemPathUtil) so XDG directories
    // (Desktop/Documents/...) and /home/<user> show localized names (桌面/文档/主目录) in the
    // sidebar tree, consistent with the workspace file view.
    QString fileName = info ? info->displayOf(DisPlayInfoType::kFileDisplayName) : "Unknown";
    if (fileName.isEmpty())
        fileName = info ? info->fileName() : "Unknown";
    QIcon folderIcon = QIcon::fromTheme("folder");

    SideBarItem *item = new SideBarItem(folderIcon, fileName, group, url);
    if (!item) {
        fmDebug() << "Failed to create new sidebar item";
        return;
    }

    ItemInfo itemInfo = ItemInfo(url, { { PropertyKey::kItemExpandable, true },
                                        { PropertyKey::kFinalUrl, url },
                                        { PropertyKey::kGroup, DefaultGroup::kDevice },
                                        { PropertyKey::kDisplayName, fileName } });
    SideBarInfoCacheMananger::instance()->addItemInfoCache(itemInfo);

    item->setExpandable(true);

    // Sub-items are not editable and not draggable.
    Qt::ItemFlags flags = item->flags();
    flags &= ~Qt::ItemIsEditable;
    flags &= ~Qt::ItemIsDragEnabled;
    item->setFlags(flags);

    // Insert in sorted order using the global FileNameSorter (numeric-aware ICU
    // collation) so that the sidebar sub-tree ordering matches the file view,
    // title bar crumb completion, and desktop canvas.
    QString newName = fileName;
    int insertRow = childCount;
    for (int i = 0; i < childCount; ++i) {
        SideBarItem *childItem = dynamic_cast<SideBarItem *>(parentItem->child(i));
        if (childItem) {
            QString childName = childItem->text();
            if (dfmbase::FileNameSorter::compare(newName, childName, Qt::AscendingOrder)) {
                insertRow = i;
                break;
            }
        }
    }

    parentItem->insertRow(insertRow, item);
    setItemAccessibleName(item);
}

void SideBarModel::removeSubItem(const QModelIndex &index, const QUrl &url)
{
    if (!index.isValid()) {
        fmDebug() << "Directory not found in sidebar:" << url;
        return;
    }

    QModelIndex parentIndex = index.parent();
    if (!parentIndex.isValid()) {
        fmDebug() << "Parent index is invalid";
        return;
    }

    QStandardItem *parentItem = itemFromIndex(parentIndex);
    if (!parentItem) {
        fmDebug() << "Failed to get parent item from index";
        return;
    }

    SideBarItem *itemToRemove = itemFromIndex(index);
    if (!itemToRemove) {
        fmDebug() << "Failed to get item from index";
        return;
    }

    SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(parentItem);
    if (separatorItem) {
        // Parent is a separator (group), so only remove children of the current item.
        fmDebug() << "Parent item is a separator, removing all children of:" << url;

        int childCount = itemToRemove->rowCount();
        if (childCount > 0) {
            beginRemoveRows(index, 0, childCount - 1);
            for (int i = childCount - 1; i >= 0; --i) {
                if (auto *child = static_cast<SideBarItem *>(itemToRemove->child(i)))
                    SideBarInfoCacheMananger::instance()->removeItemInfoCache(child->url());
                itemToRemove->removeRow(i);
            }
            endRemoveRows();
            fmDebug() << "Removed" << childCount << "children from:" << url;
        }

        emit requestCollapseItem(index);
        fmDebug() << "Requested to collapse item after removed children:" << url;
    } else {
        // Parent is a regular item, remove the item itself.
        fmDebug() << "Removing item:" << url;
        SideBarInfoCacheMananger::instance()->removeItemInfoCache(url);
        parentItem->removeRow(index.row());
        fmDebug() << "Item removed from sidebar:" << url;
    }
}

void SideBarModel::setItemAccessibleName(SideBarItem *item) const
{
    if (!item)
        return;
    // Use the display text (already translated) as both objectName and accessibleName
    // for AT-SPI accessibility tree lookup.
    const QString &text = item->text();
    if (!text.isEmpty()) {
        item->setObjectName(text);
        item->setAccessibleName(text);
    }
}
