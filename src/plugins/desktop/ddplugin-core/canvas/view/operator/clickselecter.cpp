/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "clickselecter.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "utils/desktoputils.h"
#include "grid/canvasgrid.h"

DSB_D_USE_NAMESPACE

ClickSelecter::ClickSelecter(CanvasView *parent)
    : QObject(parent)
    , view(parent)
{

}

void ClickSelecter::click(const QModelIndex &index)
{
    bool ctrl = isCtrlPressed();
    bool shift = isShiftPressed();
    lastPressedIndex = index;
    toggleIndex = QModelIndex();

    if (!index.isValid()) {
        if (!ctrl && !shift)
            clear();
    } else {
        if (ctrl)
            expandSelect(index);
        else if (shift)
            continuesSelect(index);
        else
            singleSelect(index);
    }

    view->update();
}

void ClickSelecter::release(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    bool isSeleted = view->selectionModel()->isSelected(index);
    if (isSeleted && index == lastPressedIndex) {
        if (isCtrlPressed() && index == toggleIndex) {
            //! To deselect selected item if pressed on it with ctrl
            view->selectionModel()->select(index, QItemSelectionModel::Toggle);
            view->d->operState().setCurrent(QModelIndex());
        } else if (!isCtrlOrShiftPressed()) {
            //! if pressed on selected item, clear other selected items when mouse release.
            view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            //Q_ASSERT(lastPressedIndex == view->d->operState().getFocus());//检查有不同的情况。todo delete
            view->d->operState().setCurrent(lastPressedIndex);
        }
    }

    view->update();
}

void ClickSelecter::order(const QPoint &p1, const QPoint &p2, QPoint &from, QPoint &to)
{
    // the begin pos is what y is smaller than another.
    if (p1.y() < p2.y()) {
        from = p1;
        to = p2;
    } else if (p1.y() == p2.y()){
        if (p1.x() < p2.x()) {
            from = p1;
            to = p2;
        } else {
            from = p2;
            to = p1;
        }
    } else {
        from = p2;
        to = p1;
    }
}

QList<QPoint> ClickSelecter::horizontalTraversal(const QPoint &from, const QPoint &to, const QSize &gridSize)
{
    QList<QPoint> pos;
    int x = from.x();
    int y = from.y();
    for (; y <= to.y(); ++y) {
        for (; x < gridSize.width(); ++x) {
            pos.append(QPoint(x, y));
            if (y == to.y() && x == to.x())
                break;
        }
        // wrap
        x = 0;
    }

    return pos;
}

void ClickSelecter::clear()
{
    view->selectionModel()->clear();
    OperState &state = view->d->operState();
    state.setCurrent(QModelIndex());
    state.setContBegin(QModelIndex());
}

void ClickSelecter::expandSelect(const QModelIndex &index)
{
    if (view->selectionModel()->isSelected(index))
        toggleIndex = index; // to deselect it after mouse release
    else
        view->selectionModel()->select(index, QItemSelectionModel::Select);

    {
        OperState &state = view->d->operState();
        QModelIndex cur = index;
        state.setContBegin(cur);
        state.setCurrent(cur);
    }
}

void ClickSelecter::singleSelect(const QModelIndex &index)
{
    auto model = view->selectionModel();
    if (!model->isSelected(index)) {
        // single select it.
        model->select(index, QItemSelectionModel::ClearAndSelect);
    } else {
       //! index is selected, it will to clear other selection.
       //! but this action should to do in mouse release.
    }

    OperState &state = view->d->operState();
    state.setCurrent(index);
    state.setContBegin(index);
}

void ClickSelecter::continuesSelect(const QModelIndex &index)
{
    OperState &state = view->d->operState();

    auto focus = state.current();
    auto begin = state.getContBegin();
    if (begin.isValid()) {
        // use index as end
        traverseSelect(begin, index);
        state.setCurrent(index);
    } else if (focus.isValid()) {
        // there is no begin, use focus as begin and index as end
        traverseSelect(focus, index);
        state.setCurrent(index);
        state.setContBegin(focus);
    } else {
        // single select it.
        view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        state.setCurrent(index);
        state.setContBegin(index);
    }
}

void ClickSelecter::traverseSelect(const QModelIndex &from, const QModelIndex &to)
{
    auto model = view->model();
    auto item1 = model->url(from).toString();
    auto item2 = model->url(to).toString();
    if (item1.isEmpty() || item2.isEmpty()) {
        qWarning() << "invalid item" << "from:" << item1 << "to:" << item2;
        return ;
    }

    QPair<int, QPoint> pos1;
    if (!GridIns->point(item1, pos1)) {
        qWarning() << "from" << item1 << "has no pos";
        return;
    }

    QPair<int, QPoint> pos2;
    if (!GridIns->point(item2, pos2)) {
        qWarning() << "to" << item2 << "has no pos";
        return;
    }

    int num = view->screenNum();
    if (pos1.first != num || pos2.first != num) {
        qWarning() << "item pos is not in view" << num;
        return;
    }

    traverseSelect(pos1.second, pos2.second);
}

void ClickSelecter::traverseSelect(const QPoint &p1, const QPoint &p2)
{
    QPoint from;
    QPoint to;
    order(p1, p2, from, to);

    QList<QPoint> pos = horizontalTraversal(from, to, GridIns->surfaceSize(view->screenNum()));
    auto model = view->model();
    QItemSelection selection;
    for (const QPoint &p : pos) {
        auto item = GridIns->item(view->screenNum(), p);
        if (item.isEmpty())
            continue;

        auto idx = model->index(item);
        if (!idx.isValid())
            continue;

        selection.append(QItemSelectionRange(idx));
    }

    view->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}
