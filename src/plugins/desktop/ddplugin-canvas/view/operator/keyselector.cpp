// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyselector.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "utils/keyutil.h"
#include "grid/canvasgrid.h"

#include <QKeyEvent>
#include <QTimer>

using namespace ddplugin_canvas;

KeySelector::KeySelector(CanvasView *parent)
    : ClickSelector(parent)
{
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(200);
    connect(searchTimer, &QTimer::timeout, this, &KeySelector::clearSearchKey);
}

void KeySelector::keyPressed(QKeyEvent *event)
{
    // Do not allow move when hold ctrl.
    if (event->modifiers() == Qt::ControlModifier) {
        fmDebug() << "Key press ignored - Ctrl modifier held";
        return;
    }

    QPersistentModelIndex newCurrent = moveCursor(event);
    if (!newCurrent.isValid()) {
        fmDebug() << "Key press ignored - no valid cursor movement for key:" << event->key();
        return;
    }

    const OperState &state = view->d->operState();
    auto begin = state.getContBegin();
    if (event->modifiers() == Qt::ControlModifier) {
        //! ban
        incrementSelect(newCurrent);
    } else if (event->modifiers() == Qt::ShiftModifier && begin.isValid()) {
        continuesSelect(newCurrent);
    } else {
        singleSelect(newCurrent);
    }

    //! update whole view area is low performance
    //view->update();
}

QList<Qt::Key> KeySelector::filterKeys() const
{
        QList<Qt::Key> filter = {
        Qt::Key_Down,
        Qt::Key_Up,
        Qt::Key_Left,
        Qt::Key_Right,
        Qt::Key_Home,
        Qt::Key_End,
        Qt::Key_PageUp,
        Qt::Key_PageDown,
    };

    if (view->tabKeyNavigation())
        filter << Qt::Key_Tab << Qt::Key_Backtab;
    return filter;
}

void KeySelector::keyboardSearch(const QString &search)
{
    if (search.isEmpty()) {
        fmDebug() << "Empty search string - ignoring keyboard search";
        return;
    }

    if (view->d->hookIfs && view->d->hookIfs->keyboardSearch(view->screenNum(), search)) {
        fmDebug() << "Keyboard search handled by extension hook for query:" << search;
        return;
    }

    bool reverseOrder = isShiftPressed();
    searchKeys.append(search);
    QModelIndex current = view->currentIndex();
    QModelIndex index = view->d->findIndex(searchKeys, true, current, reverseOrder, !searchTimer->isActive());

    if (index.isValid()) {
        fmDebug() << "Search result found - selecting index";
        singleSelect(index);
    } else {
        fmDebug() << "No search result found for query:" << searchKeys;
    }

    searchTimer->start();
}

QPersistentModelIndex KeySelector::moveCursor(QKeyEvent *event) const
{
    QPersistentModelIndex newCurrent;
    switch (event->key()) {
    case Qt::Key_Down:
        newCurrent = view->moveCursor(CanvasView::MoveDown, event->modifiers());
        break;
    case Qt::Key_Up:
        newCurrent = view->moveCursor(CanvasView::MoveUp, event->modifiers());
        break;
    case Qt::Key_Left:
        newCurrent = view->moveCursor(CanvasView::MoveLeft, event->modifiers());
        break;
    case Qt::Key_Right:
        newCurrent = view->moveCursor(CanvasView::MoveRight, event->modifiers());
        break;
    case Qt::Key_Home:
        newCurrent = view->moveCursor(CanvasView::MoveHome, event->modifiers());
        break;
    case Qt::Key_End:
        newCurrent = view->moveCursor(CanvasView::MoveEnd, event->modifiers());
        break;
    case Qt::Key_PageUp:
        newCurrent = view->moveCursor(CanvasView::MovePageUp, event->modifiers());
        break;
    case Qt::Key_PageDown:
        newCurrent = view->moveCursor(CanvasView::MovePageDown, event->modifiers());
        break;
    case Qt::Key_Tab:
        if (view->tabKeyNavigation())
            newCurrent = view->moveCursor(CanvasView::MoveNext, event->modifiers());
        break;
    case Qt::Key_Backtab:
        if (view->tabKeyNavigation())
            newCurrent = view->moveCursor(CanvasView::MovePrevious, event->modifiers());
        break;
    }
    return newCurrent;
}

void KeySelector::singleSelect(const QModelIndex &index)
{
    auto &state = view->d->operState();
    view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    state.setCurrent(index);
    state.setContBegin(index);
}

void KeySelector::incrementSelect(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto &state = view->d->operState();
    view->selectionModel()->select(index, QItemSelectionModel::Select);
    state.setCurrent(index);
    state.setContBegin(index);
}

void KeySelector::clearSearchKey()
{
    searchKeys.clear();
}

void KeySelector::toggleSelect()
{
    auto currentSelected = view->selectionModel()->selectedIndexesCache();
    if (currentSelected.isEmpty()) {
        fmDebug() << "Toggle select ignored - no current selection";
        return;
    }

    auto m = view->model();
    int rowCount = m->rowCount(m->rootIndex());
    if (rowCount < 1) {
        fmDebug() << "Toggle select ignored - no items in model";
        return;
    }

    QItemSelection sel(m->index(0), m->index(rowCount - 1));
    view->selectionModel()->select(sel, QItemSelectionModel::Toggle);
}

