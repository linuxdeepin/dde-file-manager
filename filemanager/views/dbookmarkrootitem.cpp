#include "dbookmarkrootitem.h"
#include <QDebug>
#include "dbookmarkscene.h"
#include "dbookmarkitem.h"
#include <QMimeData>
#include <QDir>

DBookmarkRootItem::DBookmarkRootItem(DBookmarkScene *scene)
{
    m_scene = scene;
    QRectF rect = scene->sceneRect();
    setBounds(rect.x(), rect.y(), rect.width(), rect.height());
    setBackgroundEnable(false);
    setHoverBackgroundEnable(false);
    setCheckable(false);
    setPressBackgroundColor(QColor(0,0,0,0));
    connect(this, &DBookmarkRootItem::dropped, scene, &DBookmarkScene::dropped);

    m_dummyItem = new DBookmarkItem;
    m_dummyItem->setAcceptDrops(false);
    m_dummyItem->setReleaseBackgroundColor(Qt::lightGray);
    m_dummyItem->boundImageToRelease(":/icons/images/icons/bookmarks_normal_16px.svg");
}

DBookmarkItem *DBookmarkRootItem::getDummyItem()
{
    return m_dummyItem;
}

void DBookmarkRootItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    QDir dir(event->mimeData()->text());

    if(dir.exists())
    {
        DBookmarkItem * item = new DBookmarkItem;
        item->boundImageToHover(":/icons/images/icons/bookmarks_hover_16px.svg");
        item->boundImageToPress(":/icons/images/icons/bookmarks_hover_16px.svg");
        item->boundImageToRelease(":/icons/images/icons/bookmarks_normal_16px.svg");
        item->setText(dir.dirName());
        item->setUrl(dir.path());
        m_scene->clear(m_dummyItem);
        m_scene->insert(11, item);
    }
    emit dropped();
}
