#include "dhorizseparator.h"
#include <QPainter>

DHorizSeparator::DHorizSeparator(QWidget *parent) :
    QWidget(parent)
{
    setFixedHeight(2);
}

void DHorizSeparator::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    double w = width();
    double h = height();
    QPen pen(Qt::lightGray);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawLine(2, h/2, w-2, h/2);
    painter.end();
    QWidget::paintEvent(event);
}
