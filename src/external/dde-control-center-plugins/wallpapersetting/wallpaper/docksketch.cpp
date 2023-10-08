// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docksketch.h"

#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

DockSketch::DockSketch(QWidget *parent) : DBlurEffectWidget(parent)
{
    setFixedSize(224, 12);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(255, 255, 255, static_cast<int>(0.6 * 255)));
    setMaskAlpha(static_cast<int>(0.6 * 255));
    setBlurRectXRadius(4);
    setBlurRectYRadius(4);
}

void DockSketch::paintEvent(QPaintEvent *event)
{
    DBlurEffectWidget::paintEvent(event);

    QPainter pa(this);
    pa.setRenderHint(QPainter::Antialiasing);
    pa.setPen(Qt::NoPen);

    pa.setBrush(QColor(255, 255, 255, 0.5 * 255));
    pa.drawEllipse(QRect(4, 2, 8, 8));
    pa.drawRoundedRect(15, 2, 8, 8, 2, 2);
    pa.drawRoundedRect(26, 2, 8, 8, 2, 2);

    pa.drawRoundedRect(86, 2, 8, 8, 2, 2);
    pa.drawRoundedRect(97, 2, 8, 8, 2, 2);
    pa.drawRoundedRect(108, 2, 8, 8, 2, 2);
    pa.drawRoundedRect(119, 2, 8, 8, 2, 2);
    pa.drawRoundedRect(130, 2, 8, 8, 2, 2);


    pa.setBrush(QColor(0, 0, 0, 0.2 * 255));
    pa.drawRoundedRect(196, 3, 6, 6, 2, 2);
    pa.drawRoundedRect(205, 3, 6, 6, 2, 2);
    pa.drawRoundedRect(214, 3, 6, 6, 2, 2);
}
