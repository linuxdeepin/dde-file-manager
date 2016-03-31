#include "dbookmarkscene.h"
#include "dbookmarkitem.h"
#include <QDebug>
#include "dbookmarkrootitem.h"
#include "dbookmarkitemgroup.h"
#include <QGraphicsView>
#include <QMimeData>
#include <QDir>

DBookmarkScene::DBookmarkScene()
{
    m_itemGroup = new DBookmarkItemGroup;
    m_rootItem = new DBookmarkRootItem(this);
    m_itemGroup->addItem(m_rootItem);
    QGraphicsScene::addItem(m_rootItem);
}

void DBookmarkScene::addItem(DBookmarkItem *item)
{
    double h = height();
    double w = width();
    item->setPos(w/2, 15 + BOOKMARK_ITEM_HEIGHT * (items().size() - 1));
    item->setBounds(-w/2, -BOOKMARK_ITEM_HEIGHT/2, w, BOOKMARK_ITEM_HEIGHT);
    m_itemGroup->addItem(item);
    m_items.append(item);
    QGraphicsScene::addItem(item);
    connect(item, &DBookmarkItem::dropped, this, &DBookmarkScene::dropped);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_totalHeight += BOOKMARK_ITEM_HEIGHT;
    increaseSize();
}

void DBookmarkScene::insert(int index, DBookmarkItem *item)
{
    if(index > m_items.size())
        return;
    else if(index == m_items.size())
    {
        addItem(item);
        return;
    }

    double w = width();
    double dx = m_items.at(index)->x();
    double dy = m_items.at(index)->y();

    for(int i = index; i <  m_items.size(); i++)
    {
        double d1 = m_items.at(i)->x();
        double d2 = m_items.at(i)->y();
        m_items.at(i)->setPos(d1, d2 + BOOKMARK_ITEM_HEIGHT);
    }

    m_items.insert(index, item);
    item->setPos(dx, dy);
    item->setBounds(-w/2, -BOOKMARK_ITEM_HEIGHT/2, w, BOOKMARK_ITEM_HEIGHT);
    m_itemGroup->addItem(item);
    QGraphicsScene::addItem(item);
    connect(item, &DBookmarkItem::dropped, this, &DBookmarkScene::dropped);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_totalHeight += BOOKMARK_ITEM_HEIGHT;
    increaseSize();
}

void DBookmarkScene::insert(DBookmarkItem *before, DBookmarkItem *item)
{

}

void DBookmarkScene::remove(int index)
{

}

void DBookmarkScene::clear(DBookmarkItem *item)
{
    int i = m_items.indexOf(item);
    if(i < 0)
        return;
    for(int index = i; index < m_items.size(); index++)
    {
        double d1 = m_items.at(index)->x();
        double d2 = m_items.at(index)->y();
        m_items.at(index)->setPos(d1, d2 - BOOKMARK_ITEM_HEIGHT);
    }
    m_items.removeOne(item);
    m_itemGroup->removeItem(item);
    QGraphicsScene::removeItem(item);
    m_totalHeight -= BOOKMARK_ITEM_HEIGHT;
}

void DBookmarkScene::remove(DBookmarkItem *item)
{
    int i = m_items.indexOf(item);
    if(i < 0)
        return;
    for(int index = i; index < m_items.size(); index++)
    {
        double d1 = m_items.at(index)->x();
        double d2 = m_items.at(index)->y();
        m_items.at(index)->setPos(d1, d2 - BOOKMARK_ITEM_HEIGHT);
    }
    m_items.removeOne(item);
    m_itemGroup->removeItem(item);
    QGraphicsScene::removeItem(item);
    item->deleteLater();
    m_totalHeight -= BOOKMARK_ITEM_HEIGHT;
}

void DBookmarkScene::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rootItem->setBounds(x, y, w, h);
    QGraphicsScene::setSceneRect(x, y, w, h);
}

DBookmarkItemGroup *DBookmarkScene::getGroup()
{
    return m_itemGroup;
}

void DBookmarkScene::changed(const QList<QRectF> &region)
{
    qDebug() << "scene changed";
}

void DBookmarkScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    emit dragEntered();
    QGraphicsScene::dragEnterEvent(event);
    clear(m_rootItem->getDummyItem());

    QDir dir(event->mimeData()->text());

    if(dir.exists())
    {
        m_rootItem->getDummyItem()->setText(dir.dirName());
        insert(11, m_rootItem->getDummyItem());
    }
}

void DBookmarkScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    emit dragLeft();
    QGraphicsScene::dragEnterEvent(event);
    clear(m_rootItem->getDummyItem());
}

void DBookmarkScene::doDragFinished(const QPointF &point, DBookmarkItem *item)
{
    QGraphicsView * view = this->views().at(0);
    QPoint p(point.x(), point.y());
    QPoint topLeft = view->mapToGlobal(view->geometry().topLeft());
    QPoint bottomRight = view->mapToGlobal(view->geometry().bottomRight());
    QRect rect(topLeft, bottomRight);
    qDebug() << rect << point;
    if(!rect.contains(p))
    {
        remove(item);
    }
}

void DBookmarkScene::increaseSize()
{
    if(m_totalHeight > sceneRect().height() - BOOKMARK_ITEM_HEIGHT)
    {
        double w = sceneRect().width();
        double h = sceneRect().height();
        setSceneRect(0, 0, w, m_totalHeight + BOOKMARK_ITEM_HEIGHT * 2);
        views().at(0)->setGeometry(0, 0, w, m_totalHeight + BOOKMARK_ITEM_HEIGHT * 2);
    }
}
