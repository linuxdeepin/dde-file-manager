// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selecthelper.h"
#include "views/fileview.h"
#include "models/fileselectionmodel.h"
#include "models/fileviewmodel.h"

#include <dfm-base/utils/windowutils.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

SelectHelper::SelectHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
    fmDebug() << "SelectHelper created for FileView";
}

QModelIndex SelectHelper::getCurrentPressedIndex() const
{
    return currentPressedIndex;
}

void SelectHelper::click(const QModelIndex &index)
{
    fmDebug() << "SelectHelper click event - index valid:" << index.isValid() << "row:" << index.row();
    lastPressedIndex = index;
    currentPressedIndex = index;
}

void SelectHelper::release()
{
    fmDebug() << "SelectHelper release event - clearing current selection and pressed index";
    currentSelection = QItemSelection();
    currentPressedIndex = QModelIndex();
}

void SelectHelper::setSelection(const QItemSelection &selection)
{
    fmDebug() << "Setting selection - size:" << selection.size();
    currentSelection = selection;
}

void SelectHelper::selection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    if (flags == QItemSelectionModel::NoUpdate) {
        fmDebug() << "Selection with NoUpdate flag - skipping";
        return;
    }

    // select with shift
    if (WindowUtils::keyShiftIsPressed()) {
        fmDebug() << "Shift key pressed - processing shift selection";
        if (!lastPressedIndex.isValid()) {
            QItemSelection oldSelection = currentSelection;
            caculateSelection(rect, &oldSelection);
            view->selectionModel()->select(oldSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            fmDebug() << "Shift selection completed - using current selection";
            return;
        }

        const QModelIndex &index = view->indexAt(rect.bottomRight());
        if (!index.isValid()) {
            fmDebug() << "Invalid index at bottomRight for shift selection";
            return;
        }

        const QModelIndex &lastSelectedIndex = view->indexAt(rect.topLeft());
        if (!lastSelectedIndex.isValid()) {
            fmDebug() << "Invalid index at topLeft for shift selection";
            return;
        }

        view->selectionModel()->select(QItemSelection(lastSelectedIndex, index), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        fmDebug() << "Shift selection from index" << lastSelectedIndex.row() << "to" << index.row();
        return;
    }

    // select with ctrl
    if (WindowUtils::keyCtrlIsPressed()) {
        fmDebug() << "Ctrl key pressed - processing ctrl selection";
        QItemSelection oldSelection = currentSelection;
        view->selectionModel()->select(oldSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

        if (!lastPressedIndex.isValid()) {
            QItemSelection newSelection;
            caculateSelection(rect, &newSelection);

            view->selectionModel()->select(newSelection, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
            fmDebug() << "Ctrl selection completed - toggled" << newSelection.size() << "items";
            return;
        }

        const QModelIndex &lastSelectedIndex = view->indexAt(rect.topLeft());
        if (!lastSelectedIndex.isValid()) {
            fmDebug() << "Invalid index at topLeft for ctrl selection";
            return;
        }

        view->selectionModel()->select(lastSelectedIndex, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
        fmDebug() << "Ctrl selection toggled item at index:" << lastSelectedIndex.row();
        return;
    }

    QItemSelection newSelection;
    caculateSelection(rect, &newSelection);

    if (view->isIconViewMode()) {
        caculateAndSelectIndex(lastSelection, newSelection, flags);
        lastSelection = newSelection;
    } else {
        view->selectionModel()->select(newSelection, flags);
    }
}

bool SelectHelper::select(const QList<QUrl> &urls)
{
    if (urls.isEmpty()) {
        fmDebug() << "No URLs provided for selection";
        return false;
    }

    fmInfo() << "Selecting files by URLs - count:" << urls.size();

    QModelIndex firstIndex;
    QModelIndex lastIndex;
    const QModelIndex &root = view->rootIndex();
    view->selectionModel()->clearSelection();
    QList<QModelIndex> indexes {};
    QItemSelection selection;
    for (const QUrl &url : urls) {
        const QModelIndex &index = view->model()->getIndexByUrl(url);

        if (!index.isValid() || index == root) {
            fmDebug() << "Invalid or root index for URL:" << url.toString();
            continue;
        }

        selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);

        if (!firstIndex.isValid())
            firstIndex = index;

        lastIndex = index;
    }

    if (selection.indexes().isEmpty()) {
        fmWarning() << "No valid indexes found for file selection";
        return false;
    }

    view->selectionModel()->select(selection, QItemSelectionModel::Select);

    if (lastIndex.isValid()) {
        view->selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::Select);
        fmDebug() << "Set current index to last selected item";
    }

    if (firstIndex.isValid()) {
        view->scrollTo(firstIndex, QAbstractItemView::PositionAtTop);
        fmDebug() << "Scrolled to first selected item";
    }

    return true;
}

void SelectHelper::saveSelectedFilesList(const QUrl &current, const QList<QUrl> &urls)
{
    fmDebug() << "Saving selected files list - current:" << current.toString() << "count:" << urls.size();
    currentSelectedFile = current;
    selectedFiles = urls;
}

void SelectHelper::resortSelectFiles()
{
    if (selectedFiles.isEmpty() || !currentSelectedFile.isValid()) {
        fmDebug() << "No files to resort or invalid current file";
        return;
    }

    fmInfo() << "Resorting selected files - count:" << selectedFiles.size() << "current:" << currentSelectedFile.toString();

    select(selectedFiles);
    view->selectionModel()->setCurrentIndex(view->model()->getIndexByUrl(currentSelectedFile), QItemSelectionModel::Select);

    /// Clean
    currentSelectedFile = QUrl();
    selectedFiles.clear();

    fmDebug() << "Selected files resort completed and cleaned";
}

void SelectHelper::filterSelectedFiles(const QList<QUrl> &urlList)
{
    if (selectedFiles.isEmpty() || urlList.isEmpty()) {
        fmDebug() << "No files to filter - selected files empty:" << selectedFiles.isEmpty() << "filter list empty:" << urlList.isEmpty();
        return;
    }

    fmDebug() << "Filtering selected files - selected count:" << selectedFiles.size() << "filter list count:" << urlList.size();

    QList<QUrl> filteredUrls;

    // 遍历当前已选中的文件
    for (const QUrl &url : selectedFiles) {
        // 如果当前 URL 在传入的列表中存在，则保留
        if (urlList.contains(url)) {
            filteredUrls.append(url);
        }
    }

    // 更新 selectedFiles
    selectedFiles = filteredUrls;

    // 如果当前选中的文件不在过滤后的列表中，则更新 currentSelectedFile
    if (!selectedFiles.isEmpty() && !selectedFiles.contains(currentSelectedFile)) {
        currentSelectedFile = selectedFiles.first();
    } else if (selectedFiles.isEmpty()) {
        currentSelectedFile = QUrl();
        fmDebug() << "All selected files filtered out - cleared current selection";
    }
}

void SelectHelper::caculateSelection(const QRect &rect, QItemSelection *selection)
{
    if (view->isIconViewMode()) {
        caculateIconViewSelection(rect, selection);
    } else if (view->isListViewMode() || view->isTreeViewMode()) {
        caculateListViewSelection(rect, selection);
    }
}

void SelectHelper::caculateIconViewSelection(const QRect &rect, QItemSelection *selection)
{
    int itemCount = view->model()->rowCount(view->rootIndex());
    if (itemCount <= 0) {
        fmDebug() << "No items available for icon view selection calculation";
        return;
    }

    QRect actualRect(qMin(rect.left(), rect.right()),
                     qMin(rect.top(), rect.bottom()),
                     abs(rect.width()),
                     abs(rect.height()));

    const QModelIndex &sampleIndex = view->model()->index(0, 0, view->rootIndex());
    int itemHeight = view->rectForIndex(sampleIndex).height() + view->spacing() * 2;

    int iconVerticalTopMargin = 0;
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconVerticalTopMargin = DSizeModeHelper::element(kCompactIconVerticalTopMargin, kIconVerticalTopMargin);
#endif
    int firstRow = (actualRect.top() + view->verticalOffset() - iconVerticalTopMargin) / itemHeight;
    int lastRow = (actualRect.bottom() + view->verticalOffset() - iconVerticalTopMargin) / itemHeight + 1;

    int rowItemCount = view->itemCountForRow();
    int firstIndex = firstRow * rowItemCount;
    int lastIndex = qMin(lastRow * rowItemCount, itemCount);

    for (int i = firstIndex; i < lastIndex; ++i) {
        const QModelIndex &index = view->model()->index(i, 0, view->rootIndex());
        // Skip group header items - they should not be selectable
        if (isGroupHeaderIndex(index)) {
            continue;
        }
        
        if (view->indexInRect(actualRect, index)) {
            if (!selection->contains(index)) {
                QItemSelectionRange selectionRange(index);
                selection->push_back(selectionRange);
            }
        }
    }
}

void SelectHelper::caculateListViewSelection(const QRect &rect, QItemSelection *selection)
{
    QRect tmpRect = rect;

    tmpRect.translate(view->horizontalOffset(), view->verticalOffset());
    tmpRect.setCoords(qMin(tmpRect.left(), tmpRect.right()), qMin(tmpRect.top(), tmpRect.bottom()),
                      qMax(tmpRect.left(), tmpRect.right()), qMax(tmpRect.top(), tmpRect.bottom()));

    using RandeIndexList = FileView::RandeIndexList;
    using RandeIndex = FileView::RandeIndex;

    const RandeIndexList &list = view->rectContainsIndexes(tmpRect);
    for (const RandeIndex &indexRange : list) {
        // Handle each index in the range individually to skip group headers
        for (int row = indexRange.first; row <= indexRange.second; ++row) {
            const QModelIndex &index = view->model()->index(row, 0, view->rootIndex());
            // Skip group header items - they should not be selectable
            if (index.isValid() && !isGroupHeaderIndex(index)) {
                if (!selection->contains(index)) {
                    QItemSelectionRange selectionRange(index);
                    selection->append(selectionRange);
                }
            }
        }
    }
}

void SelectHelper::caculateAndSelectIndex(const QItemSelection &lastSelect, const QItemSelection &newSelect, QItemSelectionModel::SelectionFlags flags)
{
    const QModelIndexList &lastIndexes = lastSelect.indexes();
    const QModelIndexList &newIndexes = newSelect.indexes();

    if (newIndexes.count() == 1) {   // click one item
        view->selectionModel()->select(newSelect, flags);
        return;
    }

    for (const QModelIndex &index : newIndexes) {
        if (!lastIndexes.contains(index))
            view->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::NoUpdate);
    }

    for (const QModelIndex &index : lastIndexes) {
        if (!newIndexes.contains(index))
            view->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::NoUpdate);
    }
}

// Grouping-related selection methods implementation
void SelectHelper::handleGroupHeaderClick(const QModelIndex &index, Qt::KeyboardModifiers modifiers)
{
    if (!isGroupHeaderIndex(index)) {
        return;
    }
    
    QString groupKey = getGroupKeyFromIndex(index);
    if (groupKey.isEmpty()) {
        return;
    }
    
    fmDebug() << "Handling group header click for group:" << groupKey << "with modifiers:" << static_cast<int>(modifiers);
    
    if (modifiers & Qt::ControlModifier) {
        // Ctrl+Click: toggle group selection
        QList<QModelIndex> groupIndexes = getGroupFileIndexes(groupKey);
        bool allSelected = true;
        
        // Check if all files in group are selected
        for (const auto &idx : groupIndexes) {
            if (!view->selectionModel()->isSelected(idx)) {
                allSelected = false;
                break;
            }
        }
        
        // Toggle selection based on current state
        selectGroup(groupKey, !allSelected);
    } else {
        // Regular click: select entire group
        view->clearSelection();
        selectGroup(groupKey, true);
    }
}

void SelectHelper::selectGroup(const QString &groupKey, bool select)
{
    if (groupKey.isEmpty()) {
        return;
    }
    
    fmDebug() << "Selecting group:" << groupKey << "select:" << select;
    
    QList<QModelIndex> groupIndexes = getGroupFileIndexes(groupKey);
    if (groupIndexes.isEmpty()) {
        fmDebug() << "No files found in group:" << groupKey;
        return;
    }
    
    QItemSelectionModel::SelectionFlags flags = select 
        ? (QItemSelectionModel::Select | QItemSelectionModel::Rows)
        : (QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    
    for (const auto &index : groupIndexes) {
        if (isSelectableItem(index)) {
            view->selectionModel()->select(index, flags);
        }
    }
    
    fmDebug() << "Group selection completed for:" << groupKey << "files count:" << groupIndexes.size();
}

QList<QModelIndex> SelectHelper::getGroupFileIndexes(const QString &groupKey) const
{
    QList<QModelIndex> indexes;
    
    if (!view || !view->model()) {
        return indexes;
    }
    
    const auto model = view->model();
    int rowCount = model->rowCount(view->rootIndex());
    
    bool inTargetGroup = false;
    
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0, view->rootIndex());
        if (!index.isValid()) {
            continue;
        }
        
        if (isGroupHeaderIndex(index)) {
            QString currentGroupKey = getGroupKeyFromIndex(index);
            inTargetGroup = (currentGroupKey == groupKey);
        } else if (inTargetGroup && isSelectableItem(index)) {
            indexes.append(index);
        }
    }
    
    return indexes;
}

bool SelectHelper::isSelectableItem(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }
    
    // Group headers are not selectable
    if (isGroupHeaderIndex(index)) {
        return false;
    }
    
    // Check if the item has the necessary flags for selection
    Qt::ItemFlags flags = index.flags();
    return (flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled);
}

bool SelectHelper::isGroupHeaderIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return false;
    }
    
    // Check if URL starts with "group-header://"
    QUrl url = index.data(Global::kItemUrlRole).toUrl();
    return url.scheme() == "group-header";
}

QString SelectHelper::getGroupKeyFromIndex(const QModelIndex &index) const
{
    if (!isGroupHeaderIndex(index)) {
        return QString();
    }
    
    QUrl url = index.data(Global::kItemUrlRole).toUrl();
    QString groupKey = url.path(); // Remove "group-header://" prefix
    if (groupKey.startsWith("/")) {
        groupKey = groupKey.mid(1); // Remove leading slash
    }
    
    return groupKey;
}

void SelectHelper::selectGroupFiles(const QString &groupKey, bool select)
{
    // This method is an alias for selectGroup, provided for consistency
    selectGroup(groupKey, select);
}
