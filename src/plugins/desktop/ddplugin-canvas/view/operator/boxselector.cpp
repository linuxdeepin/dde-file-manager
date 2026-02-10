// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "boxselector.h"
#include "canvasmanager.h"
#include "view/canvasview_p.h"
#include "model/canvasselectionmodel.h"
#include "utils/keyutil.h"

#include <QItemSelection>
#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>
#include <QPainter>
#include <QDebug>

using namespace ddplugin_canvas;
class BoxSelectorGlobal : public BoxSelector
{
};
Q_GLOBAL_STATIC(BoxSelectorGlobal, boxSelectorGlobal)

BoxSelector *BoxSelector::instance()
{
    return boxSelectorGlobal;
}

void BoxSelector::beginSelect(const QPoint &globalPos, bool autoSelect)
{
    begin = globalPos;
    end = globalPos;
    active = true;

    if (automatic = autoSelect)
        qApp->installEventFilter(this);
}

void BoxSelector::setAcvite(bool ac)
{
    if (ac == active)
        return;

    active = ac;
    delayUpdate();
}

void BoxSelector::setBegin(const QPoint &globalPos)
{
    if (globalPos == begin)
        return;

    begin = globalPos;
    delayUpdate();
}

void BoxSelector::setEnd(const QPoint &globalPos)
{
    if (globalPos == end)
        return;

    end = globalPos;
    delayUpdate();
}

QRect BoxSelector::validRect(CanvasView *w) const
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

QRect BoxSelector::globalRect() const
{
    QRect selectRect;
    selectRect.setLeft(qMin(end.x(), begin.x()));
    selectRect.setTop(qMin(end.y(), begin.y()));
    selectRect.setRight(qMax(end.x(), begin.x()));
    selectRect.setBottom(qMax(end.y(), begin.y()));
    selectRect = selectRect.normalized();
    return selectRect;
}

QRect BoxSelector::clipRect(QRect rect, const QRect &geometry) const
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

bool BoxSelector::isBeginFrom(CanvasView *w)
{
    if (!w)
        return false;

    // the topleft point must be 0x0 after w->mapFromGlobal
    return innerGeometry(w).contains(w->mapFromGlobal(begin));
}

void BoxSelector::endSelect()
{
    if (!active)
        return;

    active = false;
    qApp->removeEventFilter(this);

    // hide rubber band if update timer is not active.
    if (!updateTimer.isActive())
        updateRubberBand();
}

BoxSelector::BoxSelector(QObject *parent)
    : QObject(parent)
{
    connect(&updateTimer, &QTimer::timeout, this, &BoxSelector::update);
    updateTimer.setSingleShot(true);
}

bool BoxSelector::eventFilter(QObject *watched, QEvent *event)
{
    if (active && qobject_cast<QWidget *>(watched)) {
        switch (event->type()) {
        case QEvent::MouseButtonRelease: {
            endSelect();
        } break;
        case QEvent::MouseMove: {
            QMouseEvent *e = dynamic_cast<QMouseEvent *>(event);
            if (Q_LIKELY(e->buttons().testFlag(Qt::LeftButton))) {
                end = e->globalPos();
                delayUpdate();
            } else {
                endSelect();
            }
        } break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void BoxSelector::delayUpdate()
{
    if (!updateTimer.isActive()) {
        int count = CanvasIns->selectionModel()->selectedIndexesCache().size();
        count = qMin(qMax(1, (count / 5)), 15);
        // update interval is more than 1ms and less than 10ms.
        updateTimer.start(count);
    }

    return;
}

QRect BoxSelector::innerGeometry(QWidget *w) const
{
    return QRect(QPoint(0, 0), w->size());
}

void BoxSelector::update()
{
    updateSelection();
    updateCurrentIndex();
    updateRubberBand();
}

#if 0
// the delay update will generate dirty area by drawing self
// becase view also uses end pos to draw rubber band that its rect is not recorded in BoxSelector
// it only directly update if using this solution. relate to ViewPainter::drawSelectRect()
void BoxSelector::updateRubberBand()
{
    auto views = CanvasIns->views();
    for (QSharedPointer<CanvasView> view : views) {
        // limit only select on single view
        if (!isBeginFrom(view.get()))
            continue;

        auto selectRect = validRect(view.get());
        if (rubberBand.isValid()) {
            view->update(selectRect.united(rubberBand).marginsAdded(QMargins(1, 1, 1, 1)));
        } else {
            view->update(selectRect.marginsAdded(QMargins(1, 1, 1, 1)));
        }

        rubberBand = selectRect;
    }

    if (!active)
        rubberBand = QRect();
}
#else
void BoxSelector::updateRubberBand()
{
    auto views = CanvasIns->views();
    for (QSharedPointer<CanvasView> view : views) {
        // limit only select on single view
        if (!isBeginFrom(view.get()))
            continue;

        auto selectRect = validRect(view.get());
        rubberBand.touch(view.get());
        rubberBand.setGeometry(selectRect);
    }

    rubberBand.setVisible(active);
}
#endif

void BoxSelector::updateSelection()
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

void BoxSelector::updateCurrentIndex()
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
            auto sels = view->selectionModel()->selectedIndexesCache();
            if (sels.size() == 1) {   // single, set it to current index.
                view->d->operState().setCurrent(sels.first());
                view->d->operState().setContBegin(sels.first());
            } else {
                view->d->operState().setCurrent(QModelIndex());
                view->d->operState().setContBegin(QModelIndex());
            }
        }
    }
}

void BoxSelector::selection(QItemSelection *newSelection)
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

void BoxSelector::selection(CanvasView *w, const QRect &rect, QItemSelection *newSelection)
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
            const QPoint gridPos(x, y);
            QString item = w->d->visualItem(gridPos);
            if (item.isEmpty())
                continue;

            auto itemRect = w->d->itemRect(QPoint(x, y));
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

RubberBand::RubberBand()
    : QWidget()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_InputMethodEnabled);
    setAutoFillBackground(false);
}

void RubberBand::touch(QWidget *w)
{
    auto old = parentWidget();
    if (old == w)
        return;

    if (old)
        disconnect(old, &QWidget::destroyed, this, &RubberBand::onParentDestroyed);

    setParent(w);

    if (w) {
        connect(w, &QWidget::destroyed, this, &RubberBand::onParentDestroyed);
        this->lower();   // set self to bottom of w;
    }

    hide();
}

void RubberBand::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QStyleOptionRubberBand opt;
    opt.initFrom(this);
    opt.shape = QRubberBand::Rectangle;
    opt.opaque = false;
    opt.rect = event->rect();
    style()->drawControl(QStyle::CE_RubberBand, &opt, &painter);
}

void RubberBand::onParentDestroyed(QObject *p)
{
    if (parentWidget() == p) {
        setParent(nullptr);
        hide();
    }
}
