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

#include "circleprogressanimatepad.h"
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QDebug>

CircleProgressAnimatePad::CircleProgressAnimatePad(QWidget *parent) : QWidget(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(3);
    initConnections();
}

void CircleProgressAnimatePad::initConnections()
{
    connect(m_timer, &QTimer::timeout, this, &CircleProgressAnimatePad::stepAngle);
}

void CircleProgressAnimatePad::startAnimation()
{
    isAnimateStarted = true;
    m_timer->start();
}

void CircleProgressAnimatePad::stopAnimation()
{
    isAnimateStarted = false;
    m_angle = 90*16;
    m_currentValue = 0;
    m_timer->stop();
    update();
}

void CircleProgressAnimatePad::stepAngle()
{
    m_angle -= 16;
    update();
}

void CircleProgressAnimatePad::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    QPen pen;

    if(isAnimateStarted)
        m_currentValue = 10;

    const QRect widgetRect = rect();
    QRect outerCircleRect = widgetRect;
    outerCircleRect.setWidth(outerCircleRect.width() - (m_lineWidth - 1) * 2);
    outerCircleRect.setHeight(outerCircleRect.height() - (m_lineWidth - 1) * 2);
    if (outerCircleRect.width() < outerCircleRect.height())
        outerCircleRect.setHeight(outerCircleRect.width());
    else
        outerCircleRect.setWidth(outerCircleRect.height());
    outerCircleRect.setTop((widgetRect.height() - outerCircleRect.height()) / 2);
    outerCircleRect.setLeft((widgetRect.width() - outerCircleRect.width()) / 2);

    const double percent = double(m_currentValue) / (m_maximumValue - m_minmumValue);
    const int offset = -percent * 16 * 360;

    pen.setWidth(m_lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(m_backgroundColor);
    painter.setPen(pen);
    painter.drawArc(outerCircleRect, m_angle, 16*360 + offset);

    pen.setColor(m_chunkColor);
    painter.setPen(pen);
    painter.drawArc(outerCircleRect, m_angle, offset);

    bool mouse_hover = testAttribute(Qt::WA_UnderMouse);

    if (mouse_hover && canPause) {
        const QIcon &icon = style()->standardIcon(isPauseState ? QStyle::SP_MediaPlay :QStyle::SP_MediaPause);

        icon.paint(&painter, rect().adjusted(15, 15, -15, -15));
    } else if (!isAnimateStarted) {//draw text
        QString text = QString::number(m_currentValue);
        QFont font;
        font.setPixelSize(m_fontSize);
        painter.setFont(font);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        QFontMetrics fm(font);
        QRect textRect = QRect(0, 0, fm.width(text), fm.height());
        textRect.moveCenter(QRect(0,0,width(), height()).center());
        painter.drawText(textRect, text);
    }
}

void CircleProgressAnimatePad::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    emit clicked();
}

void CircleProgressAnimatePad::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    update();
}

void CircleProgressAnimatePad::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    update();
}

int CircleProgressAnimatePad::fontSize() const
{
    return m_fontSize;
}

void CircleProgressAnimatePad::setFontSize(int fontSize)
{
    m_fontSize = fontSize;
}

bool CircleProgressAnimatePad::animationRunning() const
{
    return isAnimateStarted;
}

void CircleProgressAnimatePad::setPauseState(bool on)
{
    if (isPauseState == on)
        return;

    isPauseState = on;

    update();
}

void CircleProgressAnimatePad::setCanPause(bool on)
{
    if (canPause == on)
        return;

    canPause = on;

    update();
}

QColor CircleProgressAnimatePad::backgroundColor() const
{
    return m_backgroundColor;
}

void CircleProgressAnimatePad::setBackgroundColor(const QColor &backgroundColor)
{
    m_backgroundColor = backgroundColor;
}

QColor CircleProgressAnimatePad::chunkColor() const
{
    return m_chunkColor;
}

void CircleProgressAnimatePad::setChunkColor(const QColor &chunkColor)
{
    m_chunkColor = chunkColor;
}

int CircleProgressAnimatePad::angle() const
{
    return m_angle;
}

void CircleProgressAnimatePad::setAngle(int angle)
{
    m_angle = angle;
}

int CircleProgressAnimatePad::maximumValue() const
{
    return m_maximumValue;
}

void CircleProgressAnimatePad::setMaximumValue(int maximumValue)
{
    m_maximumValue = maximumValue;
}

int CircleProgressAnimatePad::minmumValue() const
{
    return m_minmumValue;
}

void CircleProgressAnimatePad::setMinmumValue(int minmumValue)
{
    m_minmumValue = minmumValue;
}

int CircleProgressAnimatePad::currentValue() const
{
    return m_currentValue;
}

void CircleProgressAnimatePad::setCurrentValue(int currentValue)
{
    m_currentValue = currentValue;
    update();
}

int CircleProgressAnimatePad::lineWidth() const
{
    return m_lineWidth;
}

void CircleProgressAnimatePad::setLineWidth(int lineWidth)
{
    m_lineWidth = lineWidth;
}
