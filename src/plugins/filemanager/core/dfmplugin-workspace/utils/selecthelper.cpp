// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selecthelper.h"
#include "views/fileview.h"
#include "models/fileselectionmodel.h"
#include "models/fileviewmodel.h"

#include <dfm-base/utils/windowutils.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

SelectHelper::SelectHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

QModelIndex SelectHelper::getCurrentPressedIndex() const
{
    return currentPressedIndex;
}

void SelectHelper::click(const QModelIndex &index)
{
    lastPressedIndex = index;
    currentPressedIndex = index;
}

void SelectHelper::release()
{
    currentSelection = QItemSelection();
    currentPressedIndex = QModelIndex();
}

void SelectHelper::setSelection(const QItemSelection &selection)
{
    currentSelection = selection;
}

void SelectHelper::selection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    if (flags == QItemSelectionModel::NoUpdate)
        return;

    // select with shift
    if (WindowUtils::keyShiftIsPressed()) {
        if (!lastPressedIndex.isValid()) {
            QItemSelection oldSelection = currentSelection;
            caculateSelection(rect, &oldSelection);
            view->selectionModel()->select(oldSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            return;
        }

        const QModelIndex &index = view->indexAt(rect.bottomRight());
        if (!index.isValid())
            return;

        const QModelIndex &lastSelectedIndex = view->indexAt(rect.topLeft());
        if (!lastSelectedIndex.isValid())
            return;

        view->selectionModel()->select(QItemSelection(lastSelectedIndex, index), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        return;
    }

    // select with ctrl
    if (WindowUtils::keyCtrlIsPressed()) {
        QItemSelection oldSelection = currentSelection;
        view->selectionModel()->select(oldSelection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

        if (!lastPressedIndex.isValid()) {
            QItemSelection newSelection;
            caculateSelection(rect, &newSelection);

            view->selectionModel()->select(newSelection, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
            return;
        }

        const QModelIndex &lastSelectedIndex = view->indexAt(rect.topLeft());
        if (!lastSelectedIndex.isValid())
            return;

        view->selectionModel()->select(lastSelectedIndex, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
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
    if (urls.isEmpty())
        return false;

    QModelIndex firstIndex;
    QModelIndex lastIndex;
    const QModelIndex &root = view->rootIndex();
    view->selectionModel()->clearSelection();
    QList<QModelIndex> indexes {};
    QItemSelection selection;
    for (const QUrl &url : urls) {
        const QModelIndex &index = view->model()->getIndexByUrl(url);

        if (!index.isValid() || index == root) {
            continue;
        }

        selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);

        if (!firstIndex.isValid())
            firstIndex = index;

        lastIndex = index;
    }

    if (selection.indexes().isEmpty())
        return false;

    view->selectionModel()->select(selection, QItemSelectionModel::Select);

    if (lastIndex.isValid())
        view->selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::Select);

    if (firstIndex.isValid())
        view->scrollTo(firstIndex, QAbstractItemView::PositionAtTop);

    return true;
}

void SelectHelper::saveSelectedFilesList(const QUrl &current, const QList<QUrl> &urls)
{
    currentSelectedFile = current;
    selectedFiles = urls;
}

void SelectHelper::resortSelectFiles()
{
    if (selectedFiles.isEmpty() || !currentSelectedFile.isValid())
            return;

    select(selectedFiles);
    view->selectionModel()->setCurrentIndex(view->model()->getIndexByUrl(currentSelectedFile), QItemSelectionModel::Select);

    /// Clean
    currentSelectedFile = QUrl();
    selectedFiles.clear();
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
    if (itemCount <= 0)
        return;

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

    const RandeIndexList &list = view->visibleIndexes(tmpRect);
    for (const RandeIndex &index : list) {
        selection->append(QItemSelectionRange(view->model()->index(index.first, 0, view->rootIndex()), view->model()->index(index.second, 0, view->rootIndex())));
    }
}

void SelectHelper::caculateAndSelectIndex(const QItemSelection &lastSelect, const QItemSelection &newSelect, QItemSelectionModel::SelectionFlags flags)
{
    const QModelIndexList &lastIndexes = lastSelect.indexes();
    const QModelIndexList &newIndexes = newSelect.indexes();

    if (newIndexes.count() == 1) {    // click one item
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
