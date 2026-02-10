// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    setFixedSize(fontMetrics().horizontalAdvance(text) + 20, fontMetrics().height());
    update();
}

void TipsWidget::refreshFont()
{
    setFixedSize(fontMetrics().horizontalAdvance(curText) + 20, fontMetrics().height());
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
