// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionframe_p.h"
#include "private/surface.h"

#include <DMenu>
#include <DGuiApplicationHelper>

#include <QMouseEvent>
#include <QAbstractItemView>
#include <QPainter>
#include <QBitmap>
#include <QPainterPath>
#include <QtMath>

static constexpr int kStretchWidth = 10;
static constexpr int kStretchHeight = 10;
static constexpr int kWidgetRoundRadius = 8;

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

CollectionFramePrivate::CollectionFramePrivate(CollectionFrame *qq)
    : q(qq)
{
    stretchArea << LeftTopRect << TopRect << RightTopRect << RightRect
                << RightBottomRect << BottomRect << LeftBottomRect << LeftRect;
    moveArea << TitleBarRect;
}

CollectionFramePrivate::~CollectionFramePrivate()
{
}

void CollectionFramePrivate::updateStretchRect()
{
    stretchRects.clear();
    stretchRects << QRect(0, 0, kStretchWidth, kStretchHeight);   // leftTopRect
    stretchRects << QRect(kStretchWidth, 0, q->width() - kStretchWidth * 2, kStretchHeight);   // topRect
    stretchRects << QRect(q->width() - kStretchWidth, 0, kStretchWidth, kStretchHeight);   // rightTopRect
    stretchRects << QRect(q->width() - kStretchWidth, kStretchHeight, kStretchWidth, q->height() - kStretchHeight * 2);   // rightRect
    stretchRects << QRect(q->width() - kStretchWidth, q->height() - kStretchHeight, kStretchWidth, kStretchHeight);   // rightBottomRect
    stretchRects << QRect(kStretchWidth, q->height() - kStretchHeight, q->width() - kStretchWidth * 2, kStretchHeight);   // bottomRect
    stretchRects << QRect(0, q->height() - kStretchHeight, kStretchWidth, kStretchHeight);   // leftBottomRect
    stretchRects << QRect(0, kStretchHeight, kStretchWidth, q->height() - kStretchHeight * 2);   // leftRect
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
    if (canStretch()) {
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
    bool tracking = canStretch() || canMove();
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

    QList<DMenu *> menus = q->findChildren<DMenu *>();
    for (auto menu : menus) {
        menu->setMouseTracking(true);
    }
}

void CollectionFramePrivate::updateFrameGeometry()
{
    QRect rect = stretchBeforRect;

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

void CollectionFramePrivate::alignToGrid()
{
    QPoint pos = q->pos();
    Surface *sur = surface();
    if (!sur)
        return;

    auto gridGeo = sur->mapToGridGeo(q->geometry());
    auto gridSize = sur->gridSize();

    if (gridGeo.x() < 0)
        gridGeo.setX(0);
    else if (gridGeo.right() >= gridSize.width())
        gridGeo.setX(gridSize.width() - gridGeo.width());

    if (gridGeo.y() < 0)
        gridGeo.setY(0);
    else if (gridGeo.bottom() >= gridSize.height())
        gridGeo.setY(gridSize.height() - gridGeo.height());

    auto rect = sur->mapToScreenGeo(gridGeo);
    pos = rect.topLeft();
    pos += { kCollectionGridMargin, kCollectionGridMargin };   // margin around collection.
    q->move(pos);
}

void CollectionFramePrivate::stretchToGrid()
{
    Surface *sur = surface();
    if (!sur)
        return;

    auto geo = q->geometry();
    // while (true) {
    int gridL = (geo.left() - sur->gridOffset().x()) / Surface::gridWidth();
    int gridR = 1 + (geo.right() - sur->gridOffset().x()) / Surface::gridWidth();
    int gridT = (geo.top() - sur->gridOffset().y()) / Surface::gridWidth();
    int gridB = 1 + (geo.bottom() - sur->gridOffset().y()) / Surface::gridWidth();

    geo = sur->mapToScreenGeo(QRect(gridL, gridT, gridR - gridL, gridB - gridT));
    geo = geo.marginsRemoved({ kCollectionGridMargin,
                               kCollectionGridMargin,
                               kCollectionGridMargin,
                               kCollectionGridMargin });

    //     if (!isIntersected(geo))
    //         break;
    // }
    q->setGeometry(geo);
}

Surface *CollectionFramePrivate::surface()
{
    auto w = dynamic_cast<Surface *>(q->parent());
    return w;
}

bool CollectionFramePrivate::canMove()
{
    return frameFeatures.testFlag(CollectionFrame::CollectionFrameMovable);
}

bool CollectionFramePrivate::canStretch()
{
    return frameFeatures.testFlag(CollectionFrame::CollectionFrameStretchable);
}

int CollectionFramePrivate::pointsDistance(const QPoint &p1, const QPoint &p2)
{
    QPoint delta = p1 - p2;
    return delta.manhattanLength();
    return qSqrt(qPow(delta.x(), 2) + qPow(delta.y(), 2));
    // maybe try manhattan length.
}

QList<QRect> CollectionFramePrivate::intersectedRects()
{
    if (!surface())
        return {};

    auto boundingRect = [](const QRect &rect) {
        return rect.marginsAdded({ kCollectionGridMargin,
                                   kCollectionGridMargin,
                                   kCollectionGridMargin,
                                   kCollectionGridMargin });
    };
    auto myRect = q->geometry();

    QMap<int, QRect> sortedRects;
    auto children = surface()->children();
    for (auto child : children) {
        CollectionFrame *frame = dynamic_cast<CollectionFrame *>(child);
        if (!frame || q == frame)
            continue;

        auto currRect = boundingRect(frame->geometry());
        if (myRect.intersects(currRect)) {
            int d = pointsDistance(myRect.center(), currRect.center());
            sortedRects.insert(d, frame->geometry());
        }
    }

    return sortedRects.values();
}

bool CollectionFramePrivate::isIntersected(const QRect &screenRect)
{
    if (!surface())
        return false;

    auto children = surface()->children();
    for (auto child : children) {
        CollectionFrame *frame = dynamic_cast<CollectionFrame *>(child);
        if (!frame || q == frame)
            continue;

        auto currRect = frame->geometry();
        if (screenRect.intersects(currRect))
            return true;
    }

    return false;
}

QRect CollectionFramePrivate::findValidArea()
{
    auto rects = intersectedRects();
    if (rects.isEmpty())
        return q->geometry();

    for (const auto &rect : rects) {
        auto pos = findValidAreaAroundRect(rect);
        if (pos.isValid())
            return pos;
    }
    return stretchBeforRect;
}

QRect CollectionFramePrivate::findValidAreaAroundRect(const QRect &centerRect)
{
    if (!surface())
        return {};

    auto myGridRect = surface()->mapToGridGeo(q->geometry());
    auto centerGridRect = surface()->mapToGridGeo(centerRect);

    int minX = centerGridRect.left() - myGridRect.width();
    int maxX = centerGridRect.right() + 1;
    int minY = centerGridRect.top() - myGridRect.height();
    int maxY = centerGridRect.bottom() + 1;

    QPoint closest(-10000, -10000);
    QRect closestRect(myGridRect);
    auto isValidPos = [this, &closestRect](const QPoint &p) {
        closestRect.moveTo(p);
        auto r = surface()->mapToScreenGeo(closestRect);
        return !isIntersected(r);
    };
    // find a closest position by every edge.
    // left. if minX != left then no valid position on left side.
    if (minX >= 0) {
        int y1, y2;
        y1 = y2 = myGridRect.topLeft().y();
        for (; !(y1 < minY && y2 > maxY); y1 -= 1, y2 += 1) {
            QPoint p1(minX, y1), p2(minX, y2);
            if (y1 >= minY && isValidPos(p1)) {
                closest = p1;
                break;
            }
            if (y2 <= maxY && isValidPos(p2)) {
                closest = p2;
                break;
            }
        }
    }

    auto closer = [this, myGridRect](const QPoint &p1, const QPoint &p2) {
        int d1 = pointsDistance(myGridRect.topLeft(), p1);
        int d2 = pointsDistance(myGridRect.topLeft(), p2);
        return d1 < d2 ? p1 : p2;
    };
    // top
    if (minY >= 0) {
        int x1, x2;
        x1 = x2 = myGridRect.topLeft().x();
        for (; !(x1 < minX && x2 > maxX); x1 -= 1, x2 += 1) {
            QPoint p1(x1, minY), p2(x2, minY);
            if (x1 >= minX && isValidPos(p1)) {
                closest = closer(closest, p1);
                break;
            }
            if (x2 <= maxX && isValidPos(p2)) {
                closest = closer(closest, p2);
                break;
            }
        }
    }

    // right
    if (maxX + myGridRect.width() <= surface()->gridSize().width()) {
        int y1, y2;
        y1 = y2 = myGridRect.topLeft().y();
        for (; !(y1 < minY && y2 > maxY); y1 -= 1, y2 += 1) {
            QPoint p1(maxX, y1), p2(maxX, y2);
            if (y1 >= minY && isValidPos(p1)) {
                closest = closer(closest, p1);
                break;
            }
            if (y2 <= maxY && isValidPos(p2)) {
                closest = closer(closest, p2);
                break;
            }
        }
    }

    // bottom
    if (maxY + myGridRect.height() <= surface()->gridSize().height()) {
        int x1, x2;
        x1 = x2 = myGridRect.topLeft().x();
        for (; !(x1 < minX && x2 > maxX); x1 -= 1, x2 += 1) {
            QPoint p1(x1, maxY), p2(x2, maxY);
            if (x1 >= minX && isValidPos(p1)) {
                closest = closer(closest, p1);
                break;
            }
            if (x2 <= maxX && isValidPos(p2)) {
                closest = closer(closest, p2);
                break;
            }
        }
    }

    if (closest.x() >= 0 && closest.y() >= 0) {
        closestRect.moveTo(closest);
        closestRect = surface()->mapToScreenGeo(closestRect);
        QRect r = q->geometry();
        r.moveTo(closestRect.topLeft());
        return r;
    }

    return {};
}

int CollectionFramePrivate::calcLeftX()
{
    int minLimitX = 0;
    int maxLimitX = stretchBeforRect.bottomRight().x() - minWidth;
    int afterX = stretchEndPoint.x() > maxLimitX ? maxLimitX : (stretchEndPoint.x() > minLimitX ? stretchEndPoint.x() : minLimitX);

    return afterX;
}

int CollectionFramePrivate::calcRightX()
{
    int minLimitX = stretchBeforRect.bottomLeft().x() + minWidth;
    int afterX = stretchEndPoint.x() > minLimitX ? stretchEndPoint.x() : minLimitX;

    QWidget *parentWidget = qobject_cast<QWidget *>(q->parent());
    if (parentWidget) {
        int maxLimitX = parentWidget->geometry().width();
        afterX = afterX > maxLimitX ? maxLimitX : afterX;
    }

    return afterX;
}

int CollectionFramePrivate::calcTopY()
{
    int minLimitY = 0;
    int maxLimitY = stretchBeforRect.bottomLeft().y() - minHeight;
    int afterY = stretchEndPoint.y() > maxLimitY ? maxLimitY : (stretchEndPoint.y() > minLimitY ? stretchEndPoint.y() : minLimitY);

    return afterY;
}

int CollectionFramePrivate::calcBottomY()
{
    int minLimitY = stretchBeforRect.topLeft().y() + minHeight;
    int afterY = stretchEndPoint.y() > minLimitY ? stretchEndPoint.y() : minLimitY;

    QWidget *parentWidget = qobject_cast<QWidget *>(q->parent());
    if (parentWidget) {
        int maxLimitY = parentWidget->geometry().height();
        afterY = afterY > maxLimitY ? maxLimitY : afterY;
    }

    return afterY;
}

CollectionFrame::CollectionFrame(QWidget *parent)
    : DFrame(parent), d(new CollectionFramePrivate(this))
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
        d->titleBarWidget->installEventFilter(this);
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

void CollectionFrame::setStretchStyle(const CollectionFrame::CollectionFrameStretchStyle &style)
{
    d->stretchStyle = style;
    d->updateMouseTrackingState();
}

CollectionFrame::CollectionFrameStretchStyle CollectionFrame::stretchStyle() const
{
    return d->stretchStyle;
}

void CollectionFrame::setStretchStep(const int step)
{
    // todo
    Q_UNUSED(step)
}

int CollectionFrame::stretchStep() const
{
    // todo
    return 0;
}

void CollectionFrame::onSizeModeChanged(const CollectionFrameSize &size)
{
    if (!d->surface())
        return;
    // top right as anchor
    auto newSize = kDefaultGridSize.value(size);
    QRect newGeo = { QPoint { 0, 0 },
                     QSize { newSize.width() * Surface::gridWidth(),
                             newSize.height() * Surface::gridWidth() } };
    newGeo = newGeo.marginsRemoved({ kCollectionGridMargin,
                                     kCollectionGridMargin,
                                     kCollectionGridMargin,
                                     kCollectionGridMargin });
    newGeo.moveTopRight(geometry().topRight());
    // do intersects.

    setGeometry(newGeo);
}

bool CollectionFrame::event(QEvent *event)
{

    return DFrame::event(event);
}

bool CollectionFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->titleBarWidget) {
        if (event->type() == QEvent::Leave) {
            this->unsetCursor();
        }
    }

    return DFrame::eventFilter(obj, event);
}

void CollectionFrame::showEvent(QShowEvent *event)
{
    if (d->canMove()) {
        d->updateMoveRect();
    }

    if (d->canStretch()) {
        d->updateStretchRect();
    }

    return DFrame::showEvent(event);
}

void CollectionFrame::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button()) {
        d->stretchBeforRect = this->geometry();

        if (d->canStretch() && d->stretchArea.contains(d->responseArea)) {
            // handle stretch
            d->frameState = CollectionFramePrivate::StretchState;
        } else if (d->canMove() && d->moveArea.contains(d->responseArea)) {
            // handle move
            d->moveStartPoint = this->mapToParent(event->pos());
            d->frameState = CollectionFramePrivate::MoveState;
            Q_EMIT dragStarted();
        } else {
            d->frameState = CollectionFramePrivate::NormalShowState;
        }

        raise();
    }
    DFrame::mousePressEvent(event);
    event->accept();
}

void CollectionFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (d->canStretch() && CollectionFramePrivate::StretchState == d->frameState) {
        d->frameState = CollectionFramePrivate::NormalShowState;
        // if (d->collisionTest()) {
        //     setGeometry(d->stretchBeforRect);
        //     return;
        // }
        d->stretchToGrid();
        d->updateStretchRect();
    }

    if (d->canMove() && CollectionFramePrivate::MoveState == d->frameState) {
        d->frameState = CollectionFramePrivate::NormalShowState;

        auto pos = d->findValidArea();
        setGeometry(pos);

        d->alignToGrid();
        d->updateMoveRect();
        Q_EMIT dragStopped();
    }

    DFrame::mouseReleaseEvent(event);
    event->accept();
}

void CollectionFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        if (d->canStretch() && CollectionFramePrivate::StretchState == d->frameState) {
            d->stretchEndPoint = this->mapToParent(event->pos());
            d->updateFrameGeometry();

            emit geometryChanged();
        } else if (d->canMove() && CollectionFramePrivate::MoveState == d->frameState) {
            QPoint movePoint = this->mapToParent(event->pos()) - d->moveStartPoint;
            d->moveStartPoint = this->mapToParent(event->pos());
            this->move(pos().x() + movePoint.x(), pos().y() + movePoint.y());

            emit geometryChanged();
        }
    } else if (event->buttons().testFlag(Qt::NoButton)) {
        d->responseArea = d->getCurrentResponseArea(event->pos());
        d->updateCursorState(d->responseArea);
    }

    DFrame::mouseMoveEvent(event);
    event->accept();
}

void CollectionFrame::resizeEvent(QResizeEvent *event)
{
    DFrame::resizeEvent(event);
    d->titleBarRect.setWidth(event->size().width());

    if (d->canStretch())
        d->updateStretchRect();

    if (d->canMove())
        d->updateMoveRect();
}

void CollectionFrame::paintEvent(QPaintEvent *event)
{
    // draw out border
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    // dark :#000000 20%, light #000000 8%
    qreal alpha = Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::DarkType
            ? 0.2
            : 0.08;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, static_cast<int>(255 * alpha)));

    const QRect rect(QPoint(0, 0), size());
    QPainterPath out;
    out.addRoundedRect(rect, kWidgetRoundRadius, kWidgetRoundRadius);
    QPainterPath in;
    in.addRoundedRect(rect.marginsRemoved(QMargins(1, 1, 1, 1)), kWidgetRoundRadius, kWidgetRoundRadius);
    p.drawPath(out - in);

    // transparent
    event->accept();
    return;
}

void CollectionFrame::focusOutEvent(QFocusEvent *event)
{
    setCursor(Qt::ArrowCursor);

    DFrame::focusOutEvent(event);
}

void CollectionFrame::initUi()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(1, 1, 1, 1);   // 1px for border
    this->setLayout(d->mainLayout);
    setContentsMargins(0, 0, 0, 0);
}
