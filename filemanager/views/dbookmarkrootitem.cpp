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
    for(int i = 0; i < urls.size(); i++)
    {
        QUrl firstUrl = urls.at(i);
        QDir dir;
        if(firstUrl.isLocalFile())
            dir.setPath(firstUrl.toLocalFile());
        else
            dir.setPath(firstUrl.toString());
        if(dir.exists())
        {
            BookMark * bm = bookmarkManager->writeIntoBookmark(dir.dirName(), QUrl::fromLocalFile(dir.path()).toString());
            DBookmarkItem * item = new DBookmarkItem(bm);
            if(m_scene->count() == DEFAULT_ITEM_COUNT)
                m_scene->addSeparator();
            m_scene->insert(DEFAULT_ITEM_COUNT + 1, item);
        }
    }
    m_scene->clear(m_dummyItem);
    emit dropped();
}
