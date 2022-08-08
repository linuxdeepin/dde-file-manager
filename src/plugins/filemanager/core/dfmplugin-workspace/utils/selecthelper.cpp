/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "selecthelper.h"
#include "views/fileview.h"
#include "dfm-base/utils/windowutils.h"
#include "models/filesortfilterproxymodel.h"
#include "models/fileselectionmodel.h"
#include "models/fileviewmodel.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

SelectHelper::SelectHelper(FileView *parent)
    : view(parent)
{
}

QModelIndex SelectHelper::getCurrentPressedIndex() const
{
    return currentPressedIndex;
}

void SelectHelper::click(const QModelIndex &index)
{
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
        if (!currentPressedIndex.isValid()) {
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

        if (!currentPressedIndex.isValid()) {
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
        view->clearSelection();
        for (const QModelIndex &index : newSelection.indexes()) {
            view->selectionModel()->select(index, QItemSelectionModel::Select);
        }
    } else {
        view->selectionModel()->select(newSelection, flags);
    }
}

void SelectHelper::select(const QList<QUrl> &urls)
{
    QList<QModelIndex> indexes {};
    for (const QUrl &url : urls) {
        const QModelIndex &index = view->model()->getIndexByUrl(url);
        indexes << index;
    }

    select(indexes);
}

void SelectHelper::select(const QList<QModelIndex> &indexes)
{
    QModelIndex firstIndex;
    QModelIndex lastIndex;

    const QModelIndex &root = view->model()->rootIndex();
    view->selectionModel()->clearSelection();
    for (const QModelIndex &index : indexes) {
        if (!index.isValid() || index == root) {
            continue;
        }

        view->selectionModel()->select(index, QItemSelectionModel::Select);

        if (!firstIndex.isValid())
            firstIndex = index;

        lastIndex = index;
    }

    if (lastIndex.isValid())
        view->selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::Select);

    if (firstIndex.isValid())
        view->scrollTo(firstIndex, QAbstractItemView::PositionAtTop);
}

void SelectHelper::caculateSelection(const QRect &rect, QItemSelection *selection)
{
    if (view->isIconViewMode()) {
        caculateIconViewSelection(rect, selection);
    } else if (view->isListViewMode()) {
        caculateListViewSelection(rect, selection);
    }
}

void SelectHelper::caculateIconViewSelection(const QRect &rect, QItemSelection *selection)
{
    int itemCount = view->model()->rowCount(view->rootIndex());
    QPoint offset(-view->horizontalOffset(), 0);
    QRect actualRect(qMin(rect.left(), rect.right()),
                     qMin(rect.top(), rect.bottom()) + view->verticalOffset(),
                     abs(rect.width()),
                     abs(rect.height()));

    for (int i = 0; i < itemCount; ++i) {
        const QModelIndex &index = view->model()->index(i, 0, view->rootIndex());
        const QRect &itemRect = view->rectForIndex(index);

        QPoint iconOffset = QPoint(kIconModeColumnPadding, kIconModeColumnPadding);
        QRect realItemRect(itemRect.topLeft() + offset + iconOffset,
                           itemRect.bottomRight() + offset - iconOffset);

        if (!(actualRect.left() > realItemRect.right() - 3
              || actualRect.top() > realItemRect.bottom() - 3
              || realItemRect.left() + 3 > actualRect.right()
              || realItemRect.top() + 3 > actualRect.bottom())) {
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
