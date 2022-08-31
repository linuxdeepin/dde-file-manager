/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "contentbackgroundwidget.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QPainter>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

ContentBackgroundWidget::ContentBackgroundWidget(QWidget *parent) : QWidget(parent)
{
    setAutoFillBackground(false);

    // default radius is 8.
    rectRadius = 8;
}

void ContentBackgroundWidget::paintEvent(QPaintEvent *event)
{
    const DPalette &dp = DApplicationHelper::instance()->palette(this);
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
