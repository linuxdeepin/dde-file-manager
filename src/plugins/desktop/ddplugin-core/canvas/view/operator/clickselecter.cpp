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
#include "view/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "utils/desktoputils.h"

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

    if (!index.isValid()) {
        if (!ctrl && !shift) {
            view->selectionModel()->clear();
            view->d->operState().setFocus(index);
        }
    } else {
        bool isSeleted = view->selectionModel()->isSelected(index);
        if (ctrl) {
            view->selectionModel()->select(index, QItemSelectionModel::Toggle);
            view->d->operState().setFocus(isSeleted ? QModelIndex() : index);
        } else if (shift) {
            //view->d->operState().setFocus(index);
        } else {
            if (!isSeleted) {
                // single select it.
                view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            } else {
               //! index is selected, it will to clear other selection.
               //! but this action should to do in mouse release.
            }
            view->d->operState().setFocus(index);
        }
    }

    view->update();
}

void ClickSelecter::release(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    //! if pressed on selected item, clear other selected items when mouse release.
    bool isSeleted = view->selectionModel()->isSelected(index);
    if (isSeleted && index == lastPressedIndex && !isCtrlPressed()) {
        view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        //Q_ASSERT(lastPressedIndex == view->d->operState().getFocus());//检查有不同的情况。todo delete
        view->d->operState().setFocus(lastPressedIndex);
    }

    view->update();
}
