// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
    opt.rect.adjust(-1, -1, 1, 1);
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}
