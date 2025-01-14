// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "custombutton.h"

#include <DGuiApplicationHelper>
#include <DStylePainter>
#include <DStyle>
#include <DStyleOption>

#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

CustomDIconButton::CustomDIconButton(QWidget *parent)
    : DIconButton(parent)
{
    setFlat(true);
}

CustomDIconButton::CustomDIconButton(DStyle::StandardPixmap iconType, QWidget *parent)
    : DIconButton(iconType, parent)
{
    setFlat(true);
}

void CustomDIconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    DStyleOptionButton opt;
    initStyleOption(&opt);

    if (isEnabled() && underMouse() && !isDown() && !isChecked()) {
        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
        QColor hoverColor = isDarkTheme ? QColor(255, 255, 255, 15)
                                      : QColor(0, 0, 0, 26);
        
        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(hoverColor);
        painter.drawRoundedRect(rect(), radius, radius);
    }

    DStyleHelper dstyle(style());
    dstyle.drawControl(DStyle::CE_IconButton, &opt, &painter, this);
}

CustomDToolButton::CustomDToolButton(QWidget *parent)
    : DToolButton(parent)
{
}

void CustomDToolButton::initStyleOption(QStyleOptionToolButton *option) const
{
    DToolButton::initStyleOption(option);
    if (underMouse()) {
        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
        QColor hoverColor = isDarkTheme ? QColor(255, 255, 255, 15)
                                        : QColor(0, 0, 0, 26);
        option->palette.setColor(QPalette::Button, hoverColor);
    }
}

void CustomDToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}
