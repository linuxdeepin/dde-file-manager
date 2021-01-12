/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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
