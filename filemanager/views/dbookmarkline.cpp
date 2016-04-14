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
    painter->setPen(Qt::lightGray);
    painter->drawLine(-w/2, 0 , w/2, 0);
}
