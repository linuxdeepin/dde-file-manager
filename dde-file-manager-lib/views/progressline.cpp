#include "progressline.h"
#include <QPainter>

ProgressLine::ProgressLine(QWidget *parent) : QLabel(parent)
{
    setMin(0);
    setMax(100);
    setValue(0);
}

void ProgressLine::paintEvent(QPaintEvent *event){
    QPainter p(this);
    QRect bRect;
    QRect cRect;
    qreal percent = (m_value - m_min)/(m_max - m_min);
    int hue = 188 - 188*percent;

    bRect.setSize(size());
    cRect.setSize(QSize(width() * percent, height()));

    QColor bColor = QColor(0,0,0,50);
    QColor cColor;
    cColor.setHsv(hue, 255, 233);

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
    update();
}

qreal ProgressLine::max() const
{
    return m_max;
}

void ProgressLine::setMax(const qreal &max)
{
    m_max = max;
    update();
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
