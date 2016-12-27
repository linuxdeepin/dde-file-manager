#include "circleprogressanimatepad.h"
#include <QPainter>
#include <QPen>

CircleProgressAnimatePad::CircleProgressAnimatePad(QWidget *parent) : QLabel(parent)
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

    //draw text
    if(!isAnimateStarted){
        QString text = QString::number(m_currentValue);
        QFont font;
        font.setPixelSize(m_fontSize);
        painter.setFont(font);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        QFontMetrics fm(font);
        QRect textRect = QRect(0, 0, fm.width(text), m_fontSize+5);
        textRect.moveCenter(QRect(0,0,width(), height()).center());
        painter.drawText(textRect, text);
    }

    QLabel::paintEvent(event);
}

int CircleProgressAnimatePad::fontSize() const
{
    return m_fontSize;
}

void CircleProgressAnimatePad::setFontSize(int fontSize)
{
    m_fontSize = fontSize;
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
