// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "surface.h"

#include <QPainter>
#include <QPaintEvent>

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

QRect Surface::mapToScreenGeo(const QRect &gridGeo)
{
    int gridOffsetX = gridOffset().x();
    int gridOffsetY = gridOffset().y();

    auto screenPos = QPoint { gridGeo.x() * cellWidth() + gridOffsetX,
                              gridGeo.y() * cellWidth() + gridOffsetY };
    auto screenSize = QSize { gridGeo.width() * cellWidth(),
                              gridGeo.height() * cellWidth() };
    return { screenPos, screenSize };
}

QRect Surface::mapToGridGeo(const QRect &screenGeo)
{
    int gridX = (screenGeo.left() - gridOffset().x()) / cellWidth();
    int gridY = (screenGeo.top() - gridOffset().y()) / cellWidth();
    int gridW = screenGeo.width() / cellWidth() + 1;
    int gridH = screenGeo.height() / cellWidth() + 1;
    return { gridX, gridY, gridW, gridH };
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
    indicator->setGeometry(r);
    if (indicator->isHidden()) {
        indicator->lower();
        indicator->show();
    }
}

void Surface::deactivatePosIndicator()
{
    if (0 && animationEnabled()) {

    } else {
        indicator->hide();
    }
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
