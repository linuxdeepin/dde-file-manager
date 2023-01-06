// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hoverwidget.h"

HoverWidget::HoverWidget(QWidget *parent) : QWidget(parent)
{
}

HoverWidget::~HoverWidget()
{
}

void HoverWidget::enterEvent(QEvent *e)
{
    emit mouseHasEntered();
    QWidget::enterEvent(e);
}

void HoverWidget::leaveEvent(QEvent *e)
{
    emit mouseHasLeaved();
    QWidget::leaveEvent(e);
}
