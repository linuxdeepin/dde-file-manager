// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "surface.h"

#ifdef QT_DEBUG
#    include <QPainter>
#    include <QPaintEvent>
#endif

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
    return { (width() - 2 * kMargin) / gridWidth(),
             (height() - 2 * kMargin) / gridWidth() };
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
    for (int x = w - kMargin, i = 0; x > 0; x -= gridWidth(), ++i) {
        setPen(i);
        p.drawLine(QPoint { x, kMargin }, { x, (h - ((h - 2 * kMargin) % gridWidth()) - kMargin) });
    }
    for (int y = kMargin, i = 0; y < h; y += gridWidth(), ++i) {
        setPen(i);
        p.drawLine(QPoint { kMargin + (w - 2 * kMargin) % gridWidth(), y }, { w - kMargin, y });
    }
}
#endif
