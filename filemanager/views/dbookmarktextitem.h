#ifndef DBOOKMARKTEXTITEM_H
#define DBOOKMARKTEXTITEM_H

#include <QGraphicsTextItem>

class DBookmarkTextItem : public QGraphicsTextItem
{
public:
    DBookmarkTextItem();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // DBOOKMARKTEXTITEM_H
