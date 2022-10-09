/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "cover.h"

#define RADIUS 8

Cover::Cover(QWidget *parent)
    : DLabel("", parent)
{
    QWidget::setAttribute(Qt::WA_TranslucentBackground, true);
}

void Cover::setCoverPixmap(const QPixmap &pixmap)
{
    background = pixmap;
    update();
}

void Cover::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

    QRect backgroundRect = rect();
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(backgroundRect, RADIUS, RADIUS);
    painter.setClipPath(backgroundPath);
    painter.setPen(Qt::NoPen);
    if (background.isNull())
        painter.fillPath(backgroundPath, QColor(255, 255, 255, 255));
    else
        painter.drawPixmap(backgroundRect, background);

    painter.setBrush(Qt::NoBrush);
    QPen borderPen(QColor(0, 0, 0, 20));
    borderPen.setWidthF(2);
    painter.setPen(borderPen);
    painter.drawRoundedRect(backgroundRect, RADIUS, RADIUS);
}
