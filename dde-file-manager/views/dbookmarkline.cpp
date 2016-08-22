#include "dbookmarkline.h"

DBookmarkLine::DBookmarkLine()
{
    setDefaultItem(true);
    setAcceptDrops(false);
    setObjectName("DBookmarkLine");
}

void DBookmarkLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    double w = boundWidth();
    double h = boundHeight();
    painter->setPen(QColor(0, 0, 0, 25));
    if(!isTightModel())
        painter->drawLine(0, h/2 , w, h/2);
    else
        painter->drawLine(0, h/2 , 60, h/2);
}
