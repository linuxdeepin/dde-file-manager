// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "radioframe.h"

#include <DPalette>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QPainterPath>

using namespace dfmplugin_vault;
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

RadioFrame::RadioFrame(QFrame *parent)
    : QFrame(parent)
{
    DPalette pal;
    QColor color;
    color.setRgbF(0.9, 0.9, 0.9, 0.03);
    pal.setColor(DPalette::Light, color);
    this->setPalette(pal);
}

void RadioFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);   // 反锯齿;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        painter.setBrush(QBrush(QColor("#4c252525")));
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        QColor color;
        color.setRgbF(0.0, 0.0, 0.0, 0.03);
        painter.setBrush(QBrush(color));
    }

    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);
    painter.drawRoundedRect(rect, 8, 8);
    //也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 8, 8);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 8, 8);
        painter.drawPath(painterPath);
    }
    QFrame::paintEvent(event);
}
