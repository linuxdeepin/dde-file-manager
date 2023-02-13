// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
