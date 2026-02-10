// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectionframe_p.h"
#include "private/surface.h"

#include <dfm-base/utils/windowutils.h>
#include <dfm-base/dfm_desktop_defines.h>

#include <DMenu>
#include <DGuiApplicationHelper>

#include <QMouseEvent>
#include <QAbstractItemView>
#include <QPainter>
#include <QBitmap>
#include <QPainterPath>
#include <QtMath>
#include <QPropertyAnimation>
#include <QScreen>

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
    // NOTE: do not change the order!
    stretchRects << QRect(0, 0, kCollectionStretchThreshold, q->height());   // leftRect
    stretchRects << QRect(0, 0, q->width(), kCollectionStretchThreshold);   // topRect
    stretchRects << QRect(q->width() - kCollectionStretchThreshold, 0, kCollectionStretchThreshold, q->height());   // rightRect
    stretchRects << QRect(0, q->height() - kCollectionStretchThreshold, q->width(), kCollectionStretchThreshold);   // bottomRect
}

void CollectionFramePrivate::updateMoveRect()
{
    if (!titleBarWidget)
        return;

    titleBarRect = titleBarWidget->geometry();
}

CollectionFramePrivate::ResponseArea CollectionFramePrivate::getCurrentResponseArea(const QPoint &pos) const
{
    ResponseArea area = ResponseArea(0);
    if (stretchRects.count() > 0 && stretchRects[0].contains(pos))   //left
        area = LeftRect;
    if (stretchRects.count() > 1 && stretchRects[1].contains(pos))   //top
        area = ResponseArea(area | TopRect);
    if (stretchRects.count() > 2 && stretchRects[2].contains(pos))   //right
        area = ResponseArea(area | RightRect);
    if (stretchRects.count() > 3 && stretchRects[3].contains(pos))   //bottom
        area = ResponseArea(area | BottomRect);
    if (area != ResponseArea(0))
        return area;

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
    QRect rect = oldGeometry;

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

QPoint CollectionFramePrivate::moveResultRectPos(bool *validPos)
{
    QPoint pos = q->pos();
    Surface *sur = surface();
    if (!sur)
        return oldGeometry.topLeft();

    auto validRect = sur->findValidArea(q);
    if (!validRect.isValid()) {
        if (validPos)
            *validPos = false;
        return oldGeometry.topLeft();
    }
    if (validPos)
        *validPos = true;

    auto gridGeo = sur->mapToGridGeo(validRect);
    auto gridSize = sur->gridSize();

    if (gridGeo.x() < 0)
        gridGeo.setX(0);
    else if (gridGeo.right() >= gridSize.width())
        gridGeo.setX(gridSize.width() - gridGeo.width());

    if (gridGeo.y() < 0)
        gridGeo.setY(0);
    else if (gridGeo.bottom() >= gridSize.height())
        gridGeo.setY(gridSize.height() - gridGeo.height());

    auto rect = sur->mapToPixelSize(gridGeo);
    pos = rect.topLeft();
    pos += { kCollectionGridMargin, kCollectionGridMargin };   // margin around collection.

    return pos;
}

QRect CollectionFramePrivate::stretchResultRect()
{
    Surface *sur = surface();
    if (!sur)
        return oldGeometry;

    auto normalizeRect = [=](QRect screenRect) {
        if (screenRect.left() < sur->gridMargins().left())
            screenRect.setLeft(sur->gridMargins().left());
        if (screenRect.top() < sur->gridMargins().top())
            screenRect.setTop(sur->gridMargins().top());
        if (screenRect.right() > sur->width() - sur->gridMargins().right())
            screenRect.setRight(sur->width() - sur->gridMargins().right());
        if (screenRect.bottom() > sur->height() - sur->gridMargins().bottom())
            screenRect.setBottom(sur->height() - sur->gridMargins().bottom());
        auto r = sur->mapToGridGeo(screenRect);
        r = sur->mapToPixelSize(r);
        r = r.marginsRemoved({ kCollectionGridMargin,
                               kCollectionGridMargin,
                               kCollectionGridMargin,
                               kCollectionGridMargin });
        return r;
    };

    auto geo = q->geometry();
    geo = normalizeRect(geo);
    if (!sur->isIntersected(geo, q)) {
        // size not changed but pos changed may happen, do not change position in this kind of case.
        int dw = geo.width() - oldGeometry.width();
        int dh = geo.height() - oldGeometry.height();

        auto newRect = oldGeometry.adjusted(bool(responseArea & LeftRect) * -1 * dw,
                                            bool(responseArea & TopRect) * -1 * dh,
                                            bool(responseArea & RightRect) * 1 * dw,
                                            bool(responseArea & BottomRect) * 1 * dh);
        return newRect;
    }

    int dx = 1 + Surface::toCellLen(geo.width() - oldGeometry.width());
    int dy = 1 + Surface::toCellLen(geo.height() - oldGeometry.height());

    auto baseRect = oldGeometry;
    // convert shrink to expand.
    if (dx < 0) {   // make baseRect width minimum.

        // (adjust or not) * (shrink direction) * (adjust value)
        baseRect = baseRect.adjusted(bool(responseArea & LeftRect) * 1 * Surface::toPixelLen(Surface::toCellLen(baseRect.width()) - kMinCellWidth),
                                     0,
                                     bool(responseArea & RightRect) * -1 * Surface::toPixelLen((Surface::toCellLen(baseRect.width() - kMinCellWidth))),
                                     0);
        dx = 1 + Surface::toCellLen(geo.width() - baseRect.width());
    }
    if (dy < 0) {   // make baseRect height minimum.
        baseRect = baseRect.adjusted(0,
                                     bool(responseArea & TopRect) * 1 * Surface::toPixelLen(Surface::toCellLen(baseRect.height()) - kMinCellHeight),
                                     0,
                                     bool(responseArea & BottomRect) * -1 * Surface::toPixelLen(Surface::toCellLen(baseRect.height()) - kMinCellHeight));
        dy = 1 + Surface::toCellLen(geo.height() - baseRect.height());
    }

    int maxDistance = 0;
    QRect maxRect = baseRect;
    for (int i = 0; i < dx; i++) {
        for (int j = 0; j < dy; j++) {
            auto newRect = baseRect.adjusted(Surface::toPixelLen(bool(responseArea & LeftRect) * -1 * i),
                                             Surface::toPixelLen(bool(responseArea & TopRect) * -1 * j),
                                             Surface::toPixelLen(bool(responseArea & RightRect) * 1 * i),
                                             Surface::toPixelLen(bool(responseArea & BottomRect) * 1 * j));
            if (!sur->isIntersected(newRect, q) && newRect.right() < (surface()->width() - surface()->gridMargins().right())) {
                auto newSquare = i + j;
                if (newSquare >= maxDistance) {
                    maxDistance = newSquare;
                    maxRect = newRect;
                }
            } else {   // Reach the maximum y value that does not make collision, so the dy could be reduce.
                dy = j;
                break;
            }
        }
    }
    geo = maxRect;
    return geo;
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

int CollectionFramePrivate::calcLeftX()
{
    int minLimitX = 0;
    // int maxLimitX = oldGeometry.bottomRight().x() - Surface::toPixelLen(kMinCellWidth) + 2 * kCollectionGridMargin + 1;
    int maxLimitX = oldGeometry.bottomRight().x() - Surface::toPixelLen(kMinCellWidth - 1);
    int afterX = stretchEndPoint.x() > maxLimitX ? maxLimitX : (stretchEndPoint.x() > minLimitX ? stretchEndPoint.x() : minLimitX);

    return afterX;
}

int CollectionFramePrivate::calcRightX()
{
    // int minLimitX = oldGeometry.bottomLeft().x() + Surface::toPixelLen(kMinCellWidth) - 2 * kCollectionGridMargin - 1;
    int minLimitX = oldGeometry.bottomLeft().x() + Surface::toPixelLen(kMinCellWidth - 1);
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
    // int maxLimitY = oldGeometry.bottomLeft().y() - Surface::toPixelLen(kMinCellHeight) + 2 * kCollectionGridMargin + 1;
    int maxLimitY = oldGeometry.bottomLeft().y() - Surface::toPixelLen(kMinCellHeight - 1);
    int afterY = stretchEndPoint.y() > maxLimitY ? maxLimitY : (stretchEndPoint.y() > minLimitY ? stretchEndPoint.y() : minLimitY);

    return afterY;
}

int CollectionFramePrivate::calcBottomY()
{
    // int minLimitY = oldGeometry.topLeft().y() + Surface::toPixelLen(kMinCellHeight) - 2 * kCollectionGridMargin - 1;
    int minLimitY = oldGeometry.topLeft().y() + Surface::toPixelLen(kMinCellHeight - 1);
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
        d->titleBarWidget->installEventFilter(this);
    }
    d->collView = w->findChild<QWidget *>(QStringLiteral("dd_collection_view"));

    if (d->widget && d->mainLayout)
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

void CollectionFrame::adjustSizeMode(const CollectionFrameSize &size)
{
    if (!d->surface())
        return;
    // top right as anchor
    auto newSize = kDefaultCollectionSize.value(size);
    QRect newGeo = { QPoint { 0, 0 },
                     QSize { newSize.width() * Surface::cellWidth(),
                             newSize.height() * Surface::cellWidth() } };
    newGeo = newGeo.marginsRemoved({ kCollectionGridMargin,
                                     kCollectionGridMargin,
                                     kCollectionGridMargin,
                                     kCollectionGridMargin });
    newGeo.moveTopRight(geometry().topRight());

    // do intersects.
    if (d->surface()->isIntersected(newGeo, this)
        || newGeo.x() < d->surface()->gridMargins().left()
        || newGeo.bottom() > d->surface()->height() - d->surface()->gridMargins().bottom()) {
        if (Surface::animationEnabled()) {
            int leftExpand = newGeo.width() - this->width() > 0;
            int bottomExpand = newGeo.height() - this->height() > 0;
            Surface::animate({ this,
                               "geometry",
                               500,
                               QEasingCurve::Linear,
                               this->geometry(),
                               this->geometry(),
                               QPropertyAnimation::KeyValues { { 0.25, this->geometry().marginsAdded({ 5 * leftExpand, 0, 0, 5 * bottomExpand }) },
                                                               { 0.5, this->geometry() },
                                                               { 0.75, this->geometry().marginsAdded({ 5 * leftExpand, 0, 0, 5 * bottomExpand }) } } });
        }
        return;
    }

    if (Surface::animationEnabled()) {
        Surface::animate({ this,
                           "geometry",
                           300,
                           QEasingCurve::BezierSpline,
                           this->geometry(),
                           newGeo,
                           {},
                           [=] {
                               Q_EMIT geometryChanged();
                           } });
    } else {
        setGeometry(newGeo);
    }

    Q_EMIT sizeModeChanged(size);
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
        d->oldGeometry = this->geometry();

        if (d->canStretch() && d->stretchArea.contains(d->responseArea)) {
            // handle stretch
            d->frameState = CollectionFramePrivate::StretchState;

            if (d->collView)
                d->collView->setProperty(kCollectionPropertyEditing, true);
            Q_EMIT editingStatusChanged(true);
        } else if (d->canMove() && d->moveArea.contains(d->responseArea)) {
            // handle move
            d->moveStartPoint = this->mapToParent(event->pos());
            d->frameState = CollectionFramePrivate::MoveState;
            d->dragPos = event->pos();

            if (d->collView)
                d->collView->setProperty(kCollectionPropertyEditing, true);
            Q_EMIT editingStatusChanged(true);
            Q_EMIT moveStateChanged(true);
        } else {
            d->frameState = CollectionFramePrivate::NormalShowState;
        }

        raise();
    }
    Q_ASSERT(this->parent());
    d->oldSurface = dynamic_cast<Surface *>(this->parent());
    DFrame::mousePressEvent(event);
    event->accept();
}

void CollectionFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button()) {
        if (d->canStretch() && CollectionFramePrivate::StretchState == d->frameState) {
            auto result = d->stretchResultRect();
            auto resizeFinish = [=] {
                d->frameState = CollectionFramePrivate::NormalShowState;
                setGeometry(result);
                d->updateStretchRect();
                if (d->collView) d->collView->setProperty(kCollectionPropertyEditing, false);
                Q_EMIT geometryChanged();
                Q_EMIT editingStatusChanged(false);
            };

            if (Surface::animationEnabled()) {
                auto param = AnimateParams {
                    .target = this,
                    .property = "geometry",
                    .duration = 200,
                    .curve = QEasingCurve::BezierSpline,
                    .begin = this->geometry(),
                    .end = result,
                    .onFinished = resizeFinish,
                };
                Surface::animate(param);
            } else {
                resizeFinish();
            }

            // update collection size state.
            if (result != d->oldGeometry) {
                if (!d->surface())
                    return;
                auto gridSize = d->surface()->mapToGridGeo(result);
                bool foundDefault = false;
                for (auto iter = kDefaultCollectionSize.cbegin(); iter != kDefaultCollectionSize.cend(); ++iter) {
                    if (iter.value() == gridSize.size()) {
                        Q_EMIT sizeModeChanged(iter.key());
                        foundDefault = true;
                        break;
                    }
                    if (!foundDefault)
                        Q_EMIT sizeModeChanged(kFree);
                }
            }
        }

        if (d->canMove() && CollectionFramePrivate::MoveState == d->frameState) {
            bool validPos = false;
            auto pos = d->moveResultRectPos(&validPos);
            auto geometry = this->geometry();

            auto moveFinish = [=] {
                move(pos);
                d->frameState = CollectionFramePrivate::NormalShowState;
                d->updateMoveRect();
                d->surface()->update();
                if (d->collView) d->collView->setProperty(kCollectionPropertyEditing, false);
                Q_EMIT surfaceChanged(d->surface());
                Q_EMIT geometryChanged();
                Q_EMIT moveStateChanged(false);
                Q_EMIT editingStatusChanged(false);
                Q_EMIT requestDeactiveAllPredictors();
            };

            // no valid space for collection on other surface,
            // the collection should be removed from other surface
            // and get back to previous surface.
            if (!validPos && this->parent() != d->oldSurface) {
                if (Surface::animationEnabled()) {
                    // appear on old surface callback.
                    auto onVanishOnNewScreen = [=]() {
                        auto finalGeo = geometry;
                        finalGeo.moveTo(pos);
                        auto startGeo = finalGeo.marginsRemoved({ finalGeo.width() / 2,
                                                                  finalGeo.height() / 2,
                                                                  finalGeo.width() / 2,
                                                                  finalGeo.height() / 2 });
                        this->setParent(d->oldSurface);
                        Q_EMIT surfaceChanged(d->surface());
                        this->setGeometry(startGeo);
                        this->show();
                        auto params = AnimateParams {
                            .target = this,
                            .property = "geometry",
                            .duration = 200,
                            .curve = QEasingCurve::BezierSpline,
                            .begin = startGeo,
                            .end = finalGeo,
                            .onFinished = moveFinish
                        };
                        Surface::animate(params);
                    };

                    // vanish on current surface.
                    auto params = AnimateParams {
                        .target = this,
                        .property = "geometry",
                        .duration = 200,
                        .curve = QEasingCurve::BezierSpline,
                        .begin = this->geometry(),
                        .end = this->geometry().marginsRemoved({ width() / 2, height() / 2, width() / 2, height() / 2 }),
                        .onFinished = onVanishOnNewScreen
                    };
                    Surface::animate(params);
                } else {
                    setParent(d->oldSurface);
                    Q_EMIT surfaceChanged(d->surface());
                    Q_EMIT moveStateChanged(false);
                    move(pos);
                    show();
                }
            } else {
                if (Surface::animationEnabled()) {
                    auto params = AnimateParams {
                        .target = this,
                        .property = "pos",
                        .duration = 200,
                        .curve = QEasingCurve::BezierSpline,
                        .begin = this->pos(),
                        .end = pos,
                        .onFinished = moveFinish
                    };
                    Surface::animate(params);
                } else {
                    moveFinish();
                }
            }
            d->updateMoveRect();
        }
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
            if (!d->surface())
                return;

            auto screen = dfmbase::WindowUtils::cursorScreen();
            if (screen && d->surface()) {
                auto currScreenName = screen->name();
                auto parentScreenName = d->surface()->property(dfmbase::DesktopFrameProperty::kPropScreenName).toString();
                if (parentScreenName != currScreenName)
                    Q_EMIT requestChangeSurface(currScreenName, parentScreenName);
            }

            this->move(d->surface()->mapFromGlobal(QCursor::pos()) - d->dragPos);
            bool validPos = false;
            auto predictPos = d->moveResultRectPos(&validPos);
            auto rect = this->rect();
            rect.moveTopLeft(predictPos);

            Q_EMIT requestDeactiveAllPredictors();
            if (!validPos && parent() != d->oldSurface) {
                d->oldSurface->activatePosIndicator(rect);
            } else if (qAbs(predictPos.x() - this->pos().x()) >= Surface::cellWidth()
                       || qAbs(predictPos.y() - this->pos().y()) >= Surface::cellWidth()) {
                d->surface()->activatePosIndicator(rect);
            }

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
