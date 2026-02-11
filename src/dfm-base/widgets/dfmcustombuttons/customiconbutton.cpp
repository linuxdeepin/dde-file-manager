// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customiconbutton.h"
#include <dfm-base/dfm_base_global.h>

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
DFMBASE_USE_NAMESPACE

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

    if (isEnabled() && !isChecked()) {
        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);

        QColor bgColor;
        if (isDown()) {
            // 按下状态 - 20%不透明度
            bgColor = isDarkTheme ? QColor(255, 255, 255, 51)
                                  : QColor(0, 0, 0, 51);
        } else if (underMouse()) {
            // 悬浮状态 - 10%不透明度
            bgColor = isDarkTheme ? QColor(255, 255, 255, 26)
                                  : QColor(0, 0, 0, 26);
        }

        if (bgColor.isValid()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(bgColor);
            painter.drawRoundedRect(rect(), radius, radius);
        }
    }

    DStyleHelper dstyle(style());
    dstyle.drawControl(DStyle::CE_IconButton, &opt, &painter, this);
}
