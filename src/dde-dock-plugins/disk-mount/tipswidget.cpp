/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "tipswidget.h"

#include <QLabel>
#include <QPainter>

/*!
 * \class TipsWidget
 *
 * \brief TipsWidget from dde-dock
 */

TipsWidget::TipsWidget(QWidget *parent)
    : QFrame(parent)
{
}

void TipsWidget::setText(const QString &text)
{
    curText = text;
    setFixedSize(fontMetrics().width(text) + 6, fontMetrics().height());
    update();
}

void TipsWidget::refreshFont()
{
    setFixedSize(fontMetrics().width(curText) + 6, fontMetrics().height());
    update();
}

void TipsWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    refreshFont();
    QPainter painter(this);
    painter.setPen(QPen(palette().brightText(), 1));
    QTextOption option;
    option.setAlignment(Qt::AlignCenter);
    painter.drawText(rect(), curText, option);
}
