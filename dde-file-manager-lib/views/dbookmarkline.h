#ifndef DBOOKMARKLINE_H
#define DBOOKMARKLINE_H

#include "dbookmarkitem.h"

class DBookmarkLine : public DBookmarkItem
{
    Q_OBJECT

public:
    DBookmarkLine();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // DBOOKMARKLINE_H
