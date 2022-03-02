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
#include "operstate.h"
#include "view/canvasview.h"
#include "model/canvasselectionmodel.h"

DDP_CANVAS_USE_NAMESPACE

OperState::OperState(QObject *parent) : QObject(parent)
{

}

void OperState::setView(CanvasView *v)
{
    if (view = v) {
        connect(view->selectionModel(), &CanvasSelectionModel::selectionChanged,
                this, &OperState::selectionChanged);
    }
}

QModelIndex OperState::current() const
{
    return view->currentIndex();
}

void OperState::setCurrent(const QModelIndex &value)
{
    view->setCurrentIndex(value);
}

void OperState::selectionChanged()
{
    if (Q_UNLIKELY(!view))
        return;

    // reset state when selection changed in other view.
    if (auto model = view->selectionModel()) {
        // the focus is not be selected.
        auto focus = current();
        if (focus.isValid() && !model->isSelected(focus))
            setCurrent(QModelIndex());

        if (contBegin.isValid() && !model->isSelected(contBegin))
            contBegin = QModelIndex();
    }

    //! when selection changed, we need to update all view.
    //! otherwise the expanded text will partly remain.
    view->update();
}




