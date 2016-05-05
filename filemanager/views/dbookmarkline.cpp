#include "dbookmarkline.h"

DBookmarkLine::DBookmarkLine()
{
    setDefaultItem(true);
}

void DBookmarkLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    double w = boundWidth();
    double h = boundHeight();
    painter->setPen(Qt::lightGray);
    painter->drawLine(0, h/2 , w, h/2);
}
