// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customdtoolbutton.h"

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

CustomDToolButton::CustomDToolButton(QWidget *parent)
    : DToolButton(parent)
{
}

void CustomDToolButton::initStyleOption(QStyleOptionToolButton *option) const
{
    DToolButton::initStyleOption(option);
    if (isEnabled()) {
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
        option->palette.setColor(QPalette::Button, bgColor);
    }
}

void CustomDToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.rect.adjust(-1, -1, 1, 1);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}
