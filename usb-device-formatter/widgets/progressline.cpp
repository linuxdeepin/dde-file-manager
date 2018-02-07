/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "progressline.h"
#include <QPainter>

ProgressLine::ProgressLine(QWidget *parent) : QLabel(parent)
{
    setMin(0);
    setMax(100);
    setValue(0);
}

void ProgressLine::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QRect bRect;
    QRect cRect;
    qreal percent = (m_value - m_min)/(m_max - m_min);

    bRect.setSize(size());
    cRect.setSize(QSize(width() * percent, height()));

    QColor bColor = QColor(0,0,0,100);
    QColor cColor = QColor(50, 160, 255);

    p.fillRect(bRect, bColor);
    p.fillRect(cRect, cColor);

    QLabel::paintEvent(event);
}

qreal ProgressLine::min() const
{
    return m_min;
}

void ProgressLine::setMin(const qreal &min)
{
    m_min = min;
}

qreal ProgressLine::max() const
{
    return m_max;
}

void ProgressLine::setMax(const qreal &max)
{
    m_max = max;
}

qreal ProgressLine::value() const
{
    return m_value;
}

void ProgressLine::setValue(const qreal &value)
{
    m_value = value;
    update();
}
