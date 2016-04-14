#include "dbookmarkrootitem.h"
#include "dbookmarkscene.h"
#include "dbookmarkitem.h"

#include "../app/global.h"

#include "../controllers/bookmarkmanager.h"

#include <QDebug>
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
    setDefaultItem(true);
    m_dummyItem = new DBookmarkItem;
    m_dummyItem->setAcceptDrops(false);
    m_dummyItem->setReleaseBackgroundColor(Qt::lightGray);
    m_dummyItem->setPressBackgroundColor(Qt::lightGray);
    m_dummyItem->boundImageToRelease(":/icons/images/icons/bookmarks_normal_16px.svg");
}

DBookmarkItem *DBookmarkRootItem::getDummyItem()
{
    return m_dummyItem;
}

void DBookmarkRootItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if(!event->mimeData()->hasUrls())
        return;
    QList<QUrl> urls = event->mimeData()->urls();
    QUrl firstUrl = urls.at(0);
    QDir dir;
    if(firstUrl.isLocalFile())
        dir.setPath(firstUrl.toLocalFile());
    else
        dir.setPath(firstUrl.toString());
    if(dir.exists())
    {
        DBookmarkItem * item = new DBookmarkItem;
        item->boundImageToHover(":/icons/images/icons/bookmarks_hover_16px.svg");
        item->boundImageToPress(":/icons/images/icons/bookmarks_hover_16px.svg");
        item->boundImageToRelease(":/icons/images/icons/bookmarks_normal_16px.svg");
        item->setText(dir.dirName());
        item->setUrl(dir.path());
        m_scene->insert(11, item);
        bookmarkManager->writeIntoBookmark(dir.dirName(), dir.path());
    }
    m_scene->clear(m_dummyItem);
    emit dropped();
}
