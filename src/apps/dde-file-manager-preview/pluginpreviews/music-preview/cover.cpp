// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cover.h"

#include <QPainterPath>

static constexpr int kRadius { 8 };

using namespace plugin_filepreview;

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
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRect backgroundRect = rect();
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(backgroundRect, kRadius, kRadius);
    painter.setClipPath(backgroundPath);
    painter.setPen(Qt::NoPen);
    if (background.isNull())
        painter.fillPath(backgroundPath, QColor(255, 255, 255, 255));
    else
        painter.drawPixmap(backgroundRect, background);

    painter.setBrush(Qt::NoBrush);
    QPen borderPen(QColor(0, 0, 0, 20));
    borderPen.setWidthF(2.0);
    painter.setPen(borderPen);
    painter.drawRoundedRect(backgroundRect, kRadius, kRadius);
}
