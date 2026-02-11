// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operstate.h"
#include "view/canvasview.h"
#include "model/canvasselectionmodel.h"
#include "delegate/canvasitemdelegate.h"

#include <QDebug>

using namespace ddplugin_canvas;

OperState::OperState(QObject *parent) : QObject(parent)
{

}

void OperState::setView(CanvasView *v)
{
    view = v;
}

QModelIndex OperState::current() const
{
    return view->currentIndex();
}

void OperState::setCurrent(const QModelIndex &value)
{
    view->setCurrentIndex(value);
}

void OperState::updateExpendedItem()
{
    // update last expended item to clear
    if (lastExpened.isValid()) {
        // old update expended area
        auto r = view->expendedVisualRect(lastExpened);
        if (r.isValid())
            view->update(r.marginsAdded(QMargins(1, 1, 1, 1)));
    }

    // reset to null
    QModelIndex newIndex;

    // update current expended item
    if (view->itemDelegate()->mayExpand(&newIndex)) {
        // update expended area
        auto r = view->expendedVisualRect(newIndex);
        if (r.isValid())
            view->update(r.marginsAdded(QMargins(1, 1, 1, 1)));
    }

    lastExpened = newIndex;
}

void OperState::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

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

        auto indexs = model->selectedIndexesCache();
        // automatically set the first selected index to current index .
        if (!focus.isValid() && !indexs.isEmpty()) {
            setCurrent(indexs.first());
            if (!contBegin.isValid())
                contBegin = current();
        }

        //! when selection changed, we need to update the expended item.
        //! otherwise the expanded text will partly remain or not expend.
        updateExpendedItem();
    }
}
