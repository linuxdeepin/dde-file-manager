/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "collectionframe_p.h"

#include <QMouseEvent>
#include <QAbstractItemView>
#include <QPainter>
#include <QBitmap>

const static int stretchWidth = 5;
const static int stretchHeight = 5;

DWIDGET_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE

CollectionFramePrivate::CollectionFramePrivate(CollectionFrame *qq)
    : q(qq)
{

}

CollectionFramePrivate::~CollectionFramePrivate()
{

}

void CollectionFramePrivate::updateAdjustRect()
{
    stretchRects.clear();
    stretchRects << QRect(0, 0, stretchWidth, stretchHeight);   // leftTopRect
    stretchRects << QRect(stretchWidth, 0, q->width() - stretchWidth * 2, stretchHeight);   // topRect
    stretchRects << QRect(q->width() - stretchWidth, 0, stretchWidth, stretchHeight);   // rightTopRect
    stretchRects << QRect(q->width() - stretchWidth, stretchHeight, stretchWidth, q->height() - stretchHeight * 2);   // rightRect
    stretchRects << QRect(q->width() - stretchWidth, q->height() - stretchHeight, stretchWidth, stretchHeight);   // rightBottomRect
    stretchRects << QRect(stretchWidth, q->height() - stretchHeight, q->width() - stretchWidth * 2, stretchHeight);   // bottomRect
    stretchRects << QRect(0, q->height() - stretchHeight, stretchWidth, stretchHeight);   // leftBottomRect
    stretchRects << QRect(0, stretchHeight, stretchWidth, q->height() - stretchHeight * 2);   // leftRect
}

void CollectionFramePrivate::updateMoveRect()
{
    if (!titleBarWidget)
        return;

    titleBarRect = titleBarWidget->geometry();
}

CollectionFramePrivate::ResponseArea CollectionFramePrivate::getCurrentResponseArea(const QPoint &pos) const
{
    for (int i = 0; i < stretchRects.count(); ++i) {
        if (stretchRects.at(i).contains(pos))
            return ResponseArea(i);
    }

    if (titleBarRect.contains(pos))
        return TitleBarRect;

    return UnKnowRect;
}

void CollectionFramePrivate::updateCursorState(const CollectionFramePrivate::ResponseArea &stretchPlace)
{
    if (canAdjust()) {
        switch (stretchPlace) {
        case LeftTopRect:
        case RightBottomRect:
            q->setCursor(Qt::SizeFDiagCursor);
            return;
        case TopRect:
        case BottomRect:
            q->setCursor(Qt::SizeVerCursor);
            return;
        case RightTopRect:
        case LeftBottomRect:
            q->setCursor(Qt::SizeBDiagCursor);
            return;
        case RightRect:
        case LeftRect:
            q->setCursor(Qt::SizeHorCursor);
            return;
        default:
            break;
        }
    }

    if (canMove() && stretchPlace == TitleBarRect) {
        q->setCursor(Qt::SizeAllCursor);
        return;
    }

    q->setCursor(Qt::ArrowCursor);
}

void CollectionFramePrivate::updateMouseTrackingState()
{
    bool tracking = canAdjust() || canMove();
    q->setMouseTracking(tracking);

    QList<QWidget *> widgetList = q->findChildren<QWidget *>();
    for (auto w : widgetList) {
        w->setMouseTracking(tracking);
    }

    QList<QAbstractItemView *> views = q->findChildren<QAbstractItemView *>();
    for (auto view : views) {
        auto viewport = view->viewport();
        if (Q_LIKELY(viewport))
            viewport->setMouseTracking(tracking);
    }
}

void CollectionFramePrivate::updateFrameGeometry()
{
    QRect rect = adjustBeforRect;

    switch (responseArea) {
    case LeftTopRect: {
        QPoint topLeftPoint = rect.topLeft();
        topLeftPoint.setX(calcLeftX());
        topLeftPoint.setY(calcTopY());
        rect.setTopLeft(topLeftPoint);
    } break;
    case TopRect: {
        QPoint topLeftPoint = rect.topLeft();
        topLeftPoint.setY(calcTopY());
        rect.setTopLeft(topLeftPoint);
    } break;
    case RightTopRect: {
        QPoint topRightPoint = rect.topRight();
        topRightPoint.setX(calcRightX());
        topRightPoint.setY(calcTopY());
        rect.setTopRight(topRightPoint);
    } break;
    case RightRect: {
        QPoint topRightPoint = rect.topRight();
        topRightPoint.setX(calcRightX());
        rect.setTopRight(topRightPoint);
    } break;
    case RightBottomRect: {
        QPoint bottomRightPoint = rect.bottomRight();
        bottomRightPoint.setX(calcRightX());
        bottomRightPoint.setY(calcBottomY());
        rect.setBottomRight(bottomRightPoint);
    } break;
    case BottomRect: {
        QPoint bottomRightPoint = rect.bottomRight();
        bottomRightPoint.setY(calcBottomY());
        rect.setBottomRight(bottomRightPoint);
    } break;
    case LeftBottomRect: {
        QPoint bottomLeftPoint = rect.bottomLeft();
        bottomLeftPoint.setX(calcLeftX());
        bottomLeftPoint.setY(calcBottomY());
        rect.setBottomLeft(bottomLeftPoint);
    } break;
    case LeftRect: {
        QPoint bottomLeftPoint = rect.bottomLeft();
        bottomLeftPoint.setX(calcLeftX());
        rect.setBottomLeft(bottomLeftPoint);
    } break;
    case TitleBarRect:
        return;
    default:;
    }

    q->setGeometry(rect);

    // update title bar width
    titleBarRect.setWidth(rect.width());
}

bool CollectionFramePrivate::canMove()
{
    return frameFeatures.testFlag(CollectionFrame::CollectionFrameMovable);
}

bool CollectionFramePrivate::canAdjust()
{
    return frameFeatures.testFlag(CollectionFrame::CollectionFrameAdjustable)
            && CollectionFrame::CollectionFrameAdjustDisable != adjustStyle;
}

int CollectionFramePrivate::calcLeftX()
{
    int minLimitX = 0;
    int maxLimitX = adjustBeforRect.bottomRight().x() - minWidth;
    int afterX = stretchEndPoint.x() > maxLimitX ? maxLimitX : (stretchEndPoint.x() > minLimitX ? stretchEndPoint.x() : minLimitX);

    return afterX;
}

int CollectionFramePrivate::calcRightX()
{
    int minLimitX = adjustBeforRect.bottomLeft().x() + minWidth;
    int afterX = stretchEndPoint.x() > minLimitX ? stretchEndPoint.x() : minLimitX;

    QWidget *parentWidget = static_cast<QWidget *>(q->parent());
    if (parentWidget) {
        int maxLimitX = parentWidget->geometry().width();
        afterX = afterX > maxLimitX ? maxLimitX : afterX;
    }

    return afterX;
}

int CollectionFramePrivate::calcTopY()
{
    int minLimitY = 0;
    int maxLimitY = adjustBeforRect.bottomLeft().y() - minHeight;
    int afterY = stretchEndPoint.y() > maxLimitY ? maxLimitY : (stretchEndPoint.y() > minLimitY ? stretchEndPoint.y() : minLimitY);

    return afterY;
}

int CollectionFramePrivate::calcBottomY()
{
    int minLimitY = adjustBeforRect.topLeft().y() + minHeight;
    int afterY = stretchEndPoint.y() > minLimitY ? stretchEndPoint.y() : minLimitY;

    QWidget *parentWidget = static_cast<QWidget *>(q->parent());
    if (parentWidget) {
        int maxLimitY = parentWidget->geometry().height();
        afterY = afterY > maxLimitY ? maxLimitY : afterY;
    }

    return afterY;
}

CollectionFrame::CollectionFrame(QWidget *parent)
    : DFrame(parent)
    , d(new CollectionFramePrivate(this))
{
    initUi();
}

CollectionFrame::~CollectionFrame()
{

}

void CollectionFrame::setWidget(QWidget *w)
{
    if (d->widget && d->mainLayout)
        d->mainLayout->removeWidget(d->widget);

    d->widget = w;
    d->titleBarWidget = w->findChild<QWidget *>(QStringLiteral("titleBar"));
    if (d->titleBarWidget) {
        d->titleBarRect = d->titleBarWidget->geometry();
        d->minHeight = d->titleBarRect.height();
    }

    d->mainLayout->addWidget(d->widget);
}

QWidget *CollectionFrame::widget() const
{
    return d->widget;
}

void CollectionFrame::setCollectionFeatures(const CollectionFrameFeatures &features)
{
    d->frameFeatures = features;
    d->updateMouseTrackingState();
}

CollectionFrame::CollectionFrameFeatures CollectionFrame::collectionFeatures() const
{
    return d->frameFeatures;
}

void CollectionFrame::setAdjustStyle(const CollectionFrame::CollectionFrameAdjust &style)
{
    d->adjustStyle = style;
    d->updateMouseTrackingState();
}

CollectionFrame::CollectionFrameAdjust CollectionFrame::adjustStyle() const
{
    return d->adjustStyle;
}

void CollectionFrame::setAdjustStep(const int step)
{
    // todo
    Q_UNUSED(step)
}

int CollectionFrame::adjustStep() const
{
    // todo
    return 0;
}

void CollectionFrame::showEvent(QShowEvent *event)
{
    if (d->canMove()) {
        d->updateMoveRect();
    }

    if (d->canAdjust()) {
        d->updateAdjustRect();
    }

    return DFrame::showEvent(event);
}

void CollectionFrame::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button()) {
        d->mousePressed = true;

        if (d->canAdjust() && d->stretchArea.contains(d->responseArea)) {
            // 优先处理拉伸
            d->adjustBeforRect = this->geometry();
            d->frameState = CollectionFramePrivate::StretchState;
        } else if (d->canMove() && d->moveArea.contains(d->responseArea)) {
            // 再处理移动
            d->moveStartPoint = this->mapToParent(event->pos());
            d->frameState = CollectionFramePrivate::MoveState;
        } else {
            d->frameState = CollectionFramePrivate::NormalShowState;
        }
    }

    return DFrame::mousePressEvent(event);
}

void CollectionFrame::mouseReleaseEvent(QMouseEvent *event)
{
    d->mousePressed = false;
    d->frameState = CollectionFramePrivate::NormalShowState;

    if (d->canAdjust())
        d->updateAdjustRect();

    if (d->canMove())
        d->updateMoveRect();

    return DFrame::mouseReleaseEvent(event);
}

void CollectionFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (!d->mousePressed) {
        d->responseArea = d->getCurrentResponseArea(event->pos());
        d->updateCursorState(d->responseArea);
    } else {
        if (d->canAdjust() && CollectionFramePrivate::StretchState == d->frameState) {
            d->stretchEndPoint = this->mapToParent(event->pos());
            d->updateFrameGeometry();
        } else if (d->canMove() && CollectionFramePrivate::MoveState == d->frameState) {
            QPoint movePoint = this->mapToParent(event->pos()) - d->moveStartPoint;
            d->moveStartPoint = this->mapToParent(event->pos());
            this->move(pos().x() + movePoint.x(), pos().y() + movePoint.y());
        }
    }

    return DFrame::mouseMoveEvent(event);
}

void CollectionFrame::resizeEvent(QResizeEvent *event)
{
    DFrame::resizeEvent(event);

    d->titleBarRect.setWidth(event->size().width());

    if (d->canAdjust())
        d->updateAdjustRect();

    if (d->canMove())
        d->updateMoveRect();

    QBitmap roundMask(size());
    QPainter painter(&roundMask);
    painter.setBrush(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawRoundedRect(rect(), 18, 18, Qt::AbsoluteSize);
    setMask(roundMask);
}

void CollectionFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    // disable paint,because transparent background color can affect the blurring effect of child widget(view)
}

void CollectionFrame::initUi()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(d->mainLayout);
    setContentsMargins(0, 0, 0, 0);
}

