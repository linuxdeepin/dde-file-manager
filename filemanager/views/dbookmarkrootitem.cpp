#include "dbookmarkrootitem.h"
#include "dbookmarkscene.h"
#include "dbookmarkitem.h"

#include "../app/global.h"
#include "../../filemanager/app/filesignalmanager.h"

#include "../controllers/bookmarkmanager.h"
#include "../app/fmevent.h"

#include <QDebug>
#include <QMimeData>
#include <QDir>

DBookmarkRootItem::DBookmarkRootItem(DBookmarkScene *scene)
{
    m_scene = scene;
    setBackgroundEnable(false);
    setHoverBackgroundEnable(false);
    setCheckable(false);
    setPressBackgroundColor(QColor(0,0,0,0));
    setDefaultItem(true);
    setAcceptDrops(true);
    setAcceptHoverEvents(false);
}

void DBookmarkRootItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}


void DBookmarkRootItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    emit m_scene->dragLeft();
}
