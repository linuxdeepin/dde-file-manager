#ifndef DBOOKMARKROOTITEM_H
#define DBOOKMARKROOTITEM_H

#include "dbookmarkitem.h"

class DBookmarkScene;

class DBookmarkRootItem : public DBookmarkItem
{
public:
    DBookmarkRootItem(DBookmarkScene * scene);
    DBookmarkItem * getDummyItem();
protected:
    void dropEvent(QGraphicsSceneDragDropEvent *event);
private:
    DBookmarkScene * m_scene;
    DBookmarkItem * m_dummyItem;
};

#endif // DBOOKMARKROOTITEM_H
