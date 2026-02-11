// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "surface.h"

#include <QPainter>
#include <QPaintEvent>
#include <QtMath>

#include <DGuiApplicationHelper>

using namespace ddplugin_organizer;

static const int kMargin = 5;

Surface::Surface(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

QSize Surface::gridSize()
{
    return { (width() - 2 * kMargin) / cellWidth(),
             (height() - 2 * kMargin) / cellWidth() };
}

QRect Surface::mapToPixelSize(const QRect &gridGeo)
{
    int gridOffsetX = gridOffset().x();
    int gridOffsetY = gridOffset().y();

    auto screenPos = QPoint { gridGeo.x() * cellWidth() + gridOffsetX,
                              gridGeo.y() * cellWidth() + gridOffsetY };
    auto screenSize = QSize { gridGeo.width() * cellWidth(),
                              gridGeo.height() * cellWidth() };
    return { screenPos, screenSize };
}

QRect Surface::mapToGridGeo(const QRect &pixelGeo)
{
    int gridX = (pixelGeo.left() - gridOffset().x()) / cellWidth();
    int gridY = (pixelGeo.top() - gridOffset().y()) / cellWidth();
    int gridW = pixelGeo.width() / cellWidth() + 1;
    int gridH = pixelGeo.height() / cellWidth() + 1;
    return { gridX, gridY, gridW, gridH };
}

QSize Surface::mapToGridSize(const QSize &pixelSize)
{
    return { pixelSize.width() / cellWidth() + 1, pixelSize.height() / cellWidth() + 1 };
}

QPoint Surface::gridOffset()
{
    auto margins = gridMargins();
    return { margins.left(), margins.top() };
}

QMargins Surface::gridMargins()
{
    int l = width() - gridSize().width() * cellWidth() - kMargin;
    int t = kMargin;
    int r = kMargin;
    int b = height() - gridSize().height() * cellWidth() - kMargin;
    return { l, t, r, b };
}

bool Surface::animationEnabled()
{
    return true;
}

void Surface::animate(const AnimateParams &param)
{
    QPropertyAnimation *ani = new QPropertyAnimation(param.target, param.property);
    ani->setDuration(param.duration);
    ani->setEasingCurve(param.curve);
    ani->setStartValue(param.begin);
    ani->setKeyValues(param.keyVals);
    ani->setEndValue(param.end);
    ani->start(QAbstractAnimation::DeleteWhenStopped);

    if (param.onFinished)
        connect(ani, &QPropertyAnimation::finished, param.target, param.onFinished);
}

void Surface::setPositionIndicatorRect(const QRect &r)
{
    indicatorRect = r;
    update();
}

void Surface::activatePosIndicator(const QRect &r)
{
    if (!indicator)
        indicator = new ItemIndicator(this);
    if (indicator->isHidden()) {
        indicator->lower();
        indicator->show();
    }

    if (r.center() == indicator->geometry().center())
        return;
    indicator->setGeometry(r);
}

void Surface::deactivatePosIndicator()
{
    if (!indicator)
        return;
    if (0 && animationEnabled()) {

    } else {
        indicator->hide();
    }
}

int Surface::pointsDistance(const QPoint &p1, const QPoint &p2)
{
    QPoint delta = p1 - p2;
    return delta.manhattanLength();
    return qSqrt(qPow(delta.x(), 2) + qPow(delta.y(), 2));
    // maybe try manhattan length.
}

QList<QRect> Surface::intersectedRects(QWidget *wid)
{
    Q_ASSERT(wid);

    auto boundingRect = [](const QRect &rect) {
        return rect.marginsAdded({ kCollectionGridMargin,
                                   kCollectionGridMargin,
                                   kCollectionGridMargin,
                                   kCollectionGridMargin });
    };
    auto myRect = wid->geometry();
    // make sure the rect is fully in surface.
    if (myRect.left() < gridMargins().left())
        myRect.moveLeft(gridMargins().left());
    if (myRect.top() < gridMargins().top())
        myRect.moveTop(gridMargins().top());
    if (myRect.right() > width() - gridMargins().right())
        myRect.moveRight(width() - gridMargins().right());
    if (myRect.bottom() > height() - gridMargins().bottom())
        myRect.moveBottom(height() - gridMargins().bottom());

    QMap<int, QRect> sortedRects;
    auto children = this->children();
    for (auto child : children) {
        auto *frame = dynamic_cast<QWidget *>(child);
        if (!frame || wid == frame || frame->property("ignore_collision").toBool())
            continue;

        auto currRect = boundingRect(frame->geometry());
        if (myRect.intersects(currRect)) {
            int d = pointsDistance(myRect.center(), currRect.center());
            sortedRects.insert(d, frame->geometry());
        }
    }

    return sortedRects.values();
}

bool Surface::isIntersected(const QRect &screenRect, QWidget *wid)
{
    auto children = this->children();
    for (auto child : children) {
        auto *frame = dynamic_cast<QWidget *>(child);
        if (!frame || wid == frame || frame->property("ignore_collision").toBool())
            continue;

        auto currRect = frame->geometry();
        if (screenRect.intersects(currRect))
            return true;
    }

    return false;
}

QRect Surface::findValidAreaAroundRect(const QRect &centerRect, QWidget *wid)
{
    Q_ASSERT(wid);

    auto myGridRect = mapToGridGeo(wid->geometry());
    auto centerGridRect = mapToGridGeo(centerRect);

    int minX = centerGridRect.left() - myGridRect.width();
    int maxX = centerGridRect.right() + 1;
    int minY = centerGridRect.top() - myGridRect.height();
    int maxY = centerGridRect.bottom() + 1;

    QPoint closest(-10000, -10000);
    QRect closestRect(myGridRect);
    auto isValidPos = [this, &closestRect, wid](const QPoint &p) {
        closestRect.moveTo(p);
        if (closestRect.left() < 0
            || closestRect.right() >= gridSize().width()
            || closestRect.top() < 0
            || closestRect.bottom() >= gridSize().height())
            return false;
        auto r = mapToPixelSize(closestRect);
        return !isIntersected(r, wid);
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

    auto closer = [myGridRect](const QPoint &p1, const QPoint &p2) {
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
    if (maxX + myGridRect.width() <= gridSize().width()) {
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
    if (maxY + myGridRect.height() <= gridSize().height()) {
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
        closestRect = mapToPixelSize(closestRect);
        QRect r = wid->geometry();
        r.moveTo(closestRect.topLeft());
        return r;
    }

    return {};
}

QRect Surface::findValidArea(QWidget *wid)
{
    Q_ASSERT(wid);

    if (wid->width() >= width()
        || wid->height() >= height())
        return {};

    auto rects = intersectedRects(wid);
    if (rects.isEmpty())
        return wid->geometry();

    for (const auto &rect : rects) {
        auto pos = findValidAreaAroundRect(rect, wid);
        if (pos.isValid())
            return pos;
    }
    return {};
}

void Surface::paintEvent(QPaintEvent *e)
{
#ifdef QT_DEBUG
    {
        QPainter p(this);
        p.setBrush(Qt::NoBrush);
        QPen pen;
        pen.setWidth(3);
        pen.setColor(Qt::blue);
        p.setPen(pen);
        p.drawRect(QRect(QPoint(0, 0), size()));

        // draw vertical lines
        int w = this->width();
        int h = this->height();
        auto setPen = [&](int i) {
            pen.setWidth(1);
            if (i % 5 == 0 && i != 0)
                pen.setColor(QColor(255, 255, 255, 128));
            else
                pen.setColor(QColor(180, 180, 180, 128));
            p.setPen(pen);
        };
        for (int x = w - kMargin, i = 0; x > 0; x -= cellWidth(), ++i) {
            setPen(i);
            p.drawLine(QPoint { x, kMargin }, { x, (h - ((h - 2 * kMargin) % cellWidth()) - kMargin) });
        }
        for (int y = kMargin, i = 0; y < h; y += cellWidth(), ++i) {
            setPen(i);
            p.drawLine(QPoint { kMargin + (w - 2 * kMargin) % cellWidth(), y }, { w - kMargin, y });
        }
    }
#endif

    if (indicatorRect.isValid()) {
        QPainter p(this);
        p.setBrush(QColor(255, 255, 255, 128));
        p.setPen(QColor(255, 255, 255, 128));
        p.drawRoundedRect(indicatorRect, 8, 8);
    }
    QWidget::paintEvent(e);
}

ItemIndicator::ItemIndicator(QWidget *parent)
    : Dtk::Widget::DBlurEffectWidget(parent)
{
    setProperty("ignore_collision", true);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setBlurRectXRadius(8);
    setBlurRectYRadius(8);

    using namespace Dtk::Gui;
    auto setColor = [this] {
        QColor bgColor;
        if (Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::LightType)
            bgColor = QColor(255, 255, 255, static_cast<int>(0.2 * 255));   // #D2D2D2 30%
        else
            bgColor = QColor(47, 47, 47, static_cast<int>(0.2 * 255));   // #2F2F2F 30%
        setMaskColor(bgColor);
        setMaskAlpha(bgColor.alpha());
    };
    setColor();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, setColor);
}
