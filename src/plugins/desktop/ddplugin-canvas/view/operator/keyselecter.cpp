/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "keyselecter.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "utils/keyutil.h"
#include "grid/canvasgrid.h"

#include <QKeyEvent>
#include <QTimer>

DDP_CANVAS_USE_NAMESPACE

KeySelecter::KeySelecter(CanvasView *parent)
    : ClickSelecter(parent)
    , view(parent)
{
    searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(200);
    connect(searchTimer, &QTimer::timeout, this, &KeySelecter::clearSearchKey);
}

void KeySelecter::keyPressed(QKeyEvent *event)
{
    // Do not allow move when hold ctrl.
    // todo(zy) why?
    if (event->modifiers() == Qt::ControlModifier)
        return;

    QPersistentModelIndex newCurrent = moveCursor(event);
    if (!newCurrent.isValid())
        return;

    auto &state = view->d->operState();
    auto begin = state.getContBegin();
    if (event->modifiers() == Qt::ControlModifier) {
        //! ban
        incrementSelect(newCurrent);
    } else if (event->modifiers() == Qt::ShiftModifier && begin.isValid()) {
        continuesSelect(newCurrent);
    } else {
        singleSelect(newCurrent);
    }

    view->update();
}

QList<Qt::Key> KeySelecter::filterKeys() const
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

void KeySelecter::keyboardSearch(const QString &search)
{
    if (search.isEmpty())
        return;

    if (view->d->extend && view->d->extend->keyboardSearch(view->screenNum(), search))
        return;

    bool reverseOrder = isShiftPressed();
    searchKeys.append(search);
    QModelIndex current = view->currentIndex();
    QModelIndex index = view->d->findIndex(searchKeys, true, current, reverseOrder, !searchTimer->isActive());

    if (index.isValid())
        singleSelect(index);

    searchTimer->start();
}

QPersistentModelIndex KeySelecter::moveCursor(QKeyEvent *event) const
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

void KeySelecter::singleSelect(const QModelIndex &index)
{
    auto &state = view->d->operState();
    view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    state.setCurrent(index);
    state.setContBegin(index);
}

void KeySelecter::incrementSelect(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    auto &state = view->d->operState();
    view->selectionModel()->select(index, QItemSelectionModel::Select);
    state.setCurrent(index);
    state.setContBegin(index);
}

void KeySelecter::clearSearchKey()
{
    searchKeys.clear();
}

