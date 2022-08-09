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
#include "surface.h"

#ifdef QT_DEBUG
#include <QPainter>
#include <QPaintEvent>
#endif

using namespace ddplugin_organizer;

Surface::Surface(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

#ifdef QT_DEBUG
void Surface::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setBrush(Qt::NoBrush);
    QPen pen;
    pen.setWidth(3);
    pen.setColor(Qt::blue);
    p.setPen(pen);
    p.drawRect(QRect(QPoint(0, 0), size()));
    e->accept();
}
#endif
