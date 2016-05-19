#include "dbookmarktextitem.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPainter>

DBookmarkTextItem::DBookmarkTextItem()
{

}

void DBookmarkTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    //myOption.state &= ~QStyle::State_Selected;
    myOption.state &= ~QStyle::State_FocusAtBorder;
    QGraphicsTextItem::paint(painter, &myOption, widget);
}
