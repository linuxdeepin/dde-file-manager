// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clickselector.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "utils/keyutil.h"
#include "grid/canvasgrid.h"

using namespace ddplugin_canvas;

ClickSelector::ClickSelector(CanvasView *parent)
    : QObject(parent)
    , view(parent)
{

}

void ClickSelector::click(const QModelIndex &index)
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

        view->selectionModel()->hookClear();
    }
    //! update whole view area is low performance
    //view->update();
}

void ClickSelector::release(const QModelIndex &index)
{
    if (!index.isValid()) {
        fmDebug() << "Mouse release on empty area";
        return;
    }

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
    //! update whole view area is low performance
    //view->update();
}

void ClickSelector::order(const QPoint &p1, const QPoint &p2, QPoint &from, QPoint &to)
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

QList<QPoint> ClickSelector::horizontalTraversal(const QPoint &from, const QPoint &to, const QSize &gridSize)
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

void ClickSelector::clear()
{
    view->selectionModel()->clear();
    OperState &state = view->d->operState();
    state.setCurrent(QModelIndex());
    state.setContBegin(QModelIndex());
}

void ClickSelector::expandSelect(const QModelIndex &index)
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

void ClickSelector::singleSelect(const QModelIndex &index)
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

void ClickSelector::continuesSelect(const QModelIndex &index)
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

void ClickSelector::traverseSelect(const QModelIndex &from, const QModelIndex &to)
{
    auto model = view->model();
    auto item1 = model->fileUrl(from).toString();
    auto item2 = model->fileUrl(to).toString();
    if (item1.isEmpty() || item2.isEmpty()) {
        fmWarning() << "Invalid file URLs in traverse selection - from:" << item1 << "to:" << item2;
        return ;
    }

    QPair<int, QPoint> pos1;
    if (!GridIns->point(item1, pos1)) {
        fmWarning() << "Source file has no grid position:" << item1;
        return;
    }

    QPair<int, QPoint> pos2;
    if (!GridIns->point(item2, pos2)) {
        fmWarning() << "Target file has no grid position:" << item2;
        return;
    }

    int num = view->screenNum();
    if (pos1.first != num || pos2.first != num) {
        fmWarning() << "File positions not on current screen" << num << "- pos1 screen:" << pos1.first << "pos2 screen:" << pos2.first;
        return;
    }

    traverseSelect(pos1.second, pos2.second);
}

void ClickSelector::traverseSelect(const QPoint &p1, const QPoint &p2)
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
