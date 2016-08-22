#ifndef DBOOKMARKROOTITEM_H
#define DBOOKMARKROOTITEM_H

#include "dbookmarkitem.h"

#define DEFAULT_ITEM_COUNT 11

class DBookmarkScene;

class DBookmarkRootItem : public DBookmarkItem
{
public:
    DBookmarkRootItem(DBookmarkScene * scene);
protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
private:
    DBookmarkScene * m_scene;
};

#endif // DBOOKMARKROOTITEM_H
