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
}

void DBookmarkRootItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
    qDebug() << "root item drag enter";
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
        m_scene->addItem(item);
    }
}
