// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operstate.h"
#include "view/canvasview.h"
#include "model/canvasselectionmodel.h"

using namespace ddplugin_canvas;

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

        // automatically set the first selected index to current index .
        if (!focus.isValid() && !model->selectedIndexesCache().isEmpty()) {
            setCurrent(model->selectedIndexesCache().first());
            if (!contBegin.isValid())
                contBegin = current();
        }
    }

    //! when selection changed, we need to update all view.
    //! otherwise the expanded text will partly remain.
    view->update();
}




