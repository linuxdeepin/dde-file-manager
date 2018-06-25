/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dfmsidebaritemseparator.h"

#include "themeconfig.h"

#include <QPainter>

DFM_BEGIN_NAMESPACE

DFMSideBarItemSeparator::DFMSideBarItemSeparator(QWidget *parent) : QWidget(parent)
{
    // not sure
    setMinimumHeight(5);
}

void DFMSideBarItemSeparator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QColor bColor = ThemeConfig::instace()->color("BookmarkItem.DBookmarkLine", "background");
    painter.setPen(bColor);
    painter.drawLine(0, height() / 2, width(), height() / 2);
}

DFM_END_NAMESPACE
