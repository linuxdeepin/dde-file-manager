#include "dbookmarkrootitem.h"
#include "dbookmarkscene.h"
#include "dbookmarkitem.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "controllers/bookmarkmanager.h"
#include "dfmevent.h"

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
    setObjectName("BMRootItem");
}

void DBookmarkRootItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}


void DBookmarkRootItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    emit m_scene->dragLeft();
}

void DBookmarkRootItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
}
