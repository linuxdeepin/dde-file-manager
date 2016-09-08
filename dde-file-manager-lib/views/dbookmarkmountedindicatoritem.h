#ifndef DBOOKMARKMOUNTEDINDICATORITEM_H
#define DBOOKMARKMOUNTEDINDICATORITEM_H


#include "dbookmarkitem.h"

class DBookmarkMountedIndicatorItem : public DBookmarkItem
{

    Q_OBJECT

public:
    explicit DBookmarkMountedIndicatorItem(DBookmarkItem* parentItem);
    ~DBookmarkMountedIndicatorItem();

    void init();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void unmounted();

public slots:

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;

private:
    DBookmarkItem* m_parentItem = NULL;
};

#endif // DBOOKMARKMOUNTEDINDICATORITEM_H
