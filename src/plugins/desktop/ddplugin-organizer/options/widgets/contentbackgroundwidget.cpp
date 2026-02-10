// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contentbackgroundwidget.h"

#include <DPalette>
#include <DPaletteHelper>

#include <QPainter>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

ContentBackgroundWidget::ContentBackgroundWidget(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);   // 为了单击widget时，清除其他控件上的焦点

    // default radius is 8.
    rectRadius = 8;
}

void ContentBackgroundWidget::paintEvent(QPaintEvent *event)
{
    const DPalette &dp = DPaletteHelper::instance()->palette(this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(dp.brush(DPalette::ItemBackground));

    QRect roundRect(0, 0, rectRadius * 2, rectRadius * 2);
    QPainterPath path;
    int w = width();
    int h = height();

    if (edge & RoundEdge::kTop) {
        // top left
        path.moveTo(0, rectRadius);
        roundRect.moveTo(0, 0);
        path.arcTo(roundRect, 180, -90);

        // top right
        path.lineTo(w - rectRadius, 0);
        roundRect.moveTo(w - roundRect.width(), 0);
        path.arcTo(roundRect, 90, -90);
    } else {
        path.moveTo(0, 0);
        path.lineTo(w, 0);
    }

    if (edge & RoundEdge::kBottom) {
        // bottom right
        path.lineTo(w, h - rectRadius);
        roundRect.moveTo(w - roundRect.width(), h - roundRect.width());
        path.arcTo(roundRect, 0, -90);

        // bottom left
        path.lineTo(rectRadius, h);
        roundRect.moveTo(0, h - roundRect.height());
        path.arcTo(roundRect, 270, -90);
    } else {
        path.lineTo(w, h);
        path.lineTo(0, h);
    }

    path.closeSubpath();
    p.drawPath(path);

    QWidget::paintEvent(event);
}
