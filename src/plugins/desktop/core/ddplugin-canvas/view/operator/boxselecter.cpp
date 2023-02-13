// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "boxselecter.h"
#include "canvasmanager.h"
#include "view/canvasview_p.h"
#include "model/canvasselectionmodel.h"
#include "utils/keyutil.h"

#include <QItemSelection>
#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>
#include <QDebug>

using namespace ddplugin_canvas;
class BoxSelecterGlobal : public BoxSelecter{};
Q_GLOBAL_STATIC(BoxSelecterGlobal, boxSelecterGlobal)

BoxSelecter *BoxSelecter::instance()
{
    return boxSelecterGlobal;
}

void BoxSelecter::beginSelect(const QPoint &globalPos, bool autoSelect)
{
    begin = globalPos;
    end = globalPos;
    active = true;

    if (automatic = autoSelect)
        qApp->installEventFilter(this);
}

void BoxSelecter::setAcvite(bool ac)
{
    if (ac == active)
        return;

    active = ac;
    emit changed();
}

void BoxSelecter::setBegin(const QPoint &globalPos)
{
    if (globalPos == begin)
        return;

    begin = globalPos;
    emit changed();
}

void BoxSelecter::setEnd(const QPoint &globalPos)
{
    if (globalPos == end)
        return;

    end = globalPos;
    emit changed();
}

QRect BoxSelecter::validRect(CanvasView *w) const
{
    QRect selectRect;
    if (!w)
        return selectRect;

    auto rect = globalRect();
    // cover to relative rect to widget w.
    selectRect.setTopLeft(w->mapFromGlobal(rect.topLeft()));
    selectRect.setBottomRight(w->mapFromGlobal(rect.bottomRight()));

    // clip area out of widget.
    return clipRect(selectRect, innerGeometry(w));
}

QRect BoxSelecter::globalRect() const
{
    QRect selectRect;
    selectRect.setLeft(qMin(end.x(), begin.x()));
    selectRect.setTop(qMin(end.y(), begin.y()));
    selectRect.setRight(qMax(end.x(), begin.x()));
    selectRect.setBottom(qMax(end.y(), begin.y()));
    selectRect = selectRect.normalized();
    return selectRect;
}

QRect BoxSelecter::clipRect(QRect rect, const QRect &geometry) const
{
    if (rect.left() < geometry.left())
        rect.setLeft(geometry.left());

    if (rect.right() > geometry.right())
        rect.setRight(geometry.right());

    if (rect.top() < geometry.top())
        rect.setTop(geometry.top());

    if (rect.bottom() > geometry.bottom())
        rect.setBottom(geometry.bottom());

    return rect;
}

bool BoxSelecter::isBeginFrom(CanvasView *w)
{
    if (!w)
        return false;

    // the topleft point must be 0x0 after w->mapFromGlobal
    return innerGeometry(w).contains(w->mapFromGlobal(begin));
}

void BoxSelecter::endSelect()
{
    if (!active)
        return;

    active = false;
    qApp->removeEventFilter(this);
    emit changed();
}

BoxSelecter::BoxSelecter(QObject *parent) : QObject(parent)
{

}

bool BoxSelecter::eventFilter(QObject *watched, QEvent *event)
{
    if (active) {
        switch (event->type()) {
        case QEvent::MouseButtonRelease:
        {
            endSelect();
            // using queue event to prevent disrupting the event cycle
            QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
        }
            break;
        case QEvent::MouseMove:
        {
            QMouseEvent *e = dynamic_cast<QMouseEvent *>(event);
            if (Q_LIKELY(e->buttons().testFlag(Qt::LeftButton))) {
                end = e->globalPos();
                updateSelection();
                updateCurrentIndex();
                QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
            } else {
                endSelect();
                // using queue event to prevent disrupting the event cycle
                QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
            }
        }
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

QRect BoxSelecter::innerGeometry(QWidget *w) const
{
    return QRect(QPoint(0, 0), w->size());
}

void BoxSelecter::updateSelection()
{
    auto selectModel = CanvasIns->selectionModel();
    Q_ASSERT(selectModel);

    QItemSelection rectSelection;
    selection(&rectSelection);

    if (isCtrlPressed())
        selectModel->select(rectSelection, QItemSelectionModel::ToggleCurrent);
    else if (isShiftPressed())
        selectModel->select(rectSelection, QItemSelectionModel::SelectCurrent);
    else
        selectModel->select(rectSelection, QItemSelectionModel::ClearAndSelect);
}

void BoxSelecter::updateCurrentIndex()
{
    auto views = CanvasIns->views();
    for (QSharedPointer<CanvasView> view : views) {
        // limit only select on single view
        if (!isBeginFrom(view.get()))
            continue;

        auto pos = view->mapFromGlobal(end);
        auto index = view->indexAt(pos);
        if (index.isValid()) {
            // for shift and focus
            auto cur = view->selectionModel()->isSelected(index) ? index : QModelIndex();
            // box selection need update focus and shift begin.
            view->d->operState().setCurrent(cur);
            view->d->operState().setContBegin(cur);
        } else {
            // there is no item at end pos. reset all focus index.
            // we maybe need to find a greater method to process it.
            auto sels =  view->selectionModel()->selectedIndexesCache();
            if (sels.size() == 1) { // single, set it to current index.
                view->d->operState().setCurrent(sels.first());
                view->d->operState().setContBegin(sels.first());
            } else {
                view->d->operState().setCurrent(QModelIndex());
                view->d->operState().setContBegin(QModelIndex());
            }
        }
    }
}

void BoxSelecter::selection(QItemSelection *newSelection)
{
    QItemSelection allSelection;
    auto views = CanvasIns->views();
    for (QSharedPointer<CanvasView> view : views) {
        // limit only select on single view
        if (!isBeginFrom(view.get()))
            continue;

        auto selectRect = validRect(view.get());
        QItemSelection rectSelection;
        selection(view.get(), selectRect, &rectSelection);
        allSelection.merge(rectSelection, QItemSelectionModel::Select);
    }

    *newSelection = allSelection;
}

void BoxSelecter::selection(CanvasView *w, const QRect &rect, QItemSelection *newSelection)
{
    if (!w || !newSelection || !rect.isValid())
        return;

    auto model = w->model();
    if (!model)
        return;

    QItemSelection rectSelection;
    auto topLeftGridPos = w->d->gridAt(rect.topLeft());
    auto bottomRightGridPos = w->d->gridAt(rect.bottomRight());

    for (auto x = topLeftGridPos.x(); x <= bottomRightGridPos.x(); ++x) {
        for (auto y = topLeftGridPos.y(); y <= bottomRightGridPos.y(); ++y) {
            const QPoint gridPos(x ,y);
            QString item = w->d->visualItem(gridPos);
            if (item.isEmpty())
                continue;

            auto itemRect = w->d->itemRect(QPoint(x ,y));
            if (itemRect.intersects(rect)) {
                QModelIndex rowIndex = model->index(item, 0);
                QItemSelectionRange selectionRange(rowIndex);

                if (!rectSelection.contains(rowIndex))
                    rectSelection.push_back(selectionRange);
            }
        }
    }

    *newSelection = rectSelection;
}
