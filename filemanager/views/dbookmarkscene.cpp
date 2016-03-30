#include "dbookmarkscene.h"
#include "dbookmarkitem.h"
#include <QDebug>
#include "dbookmarkrootitem.h"
#include "dbookmarkitemgroup.h"

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
    item->setPos(0, -h/2 + 15 + BOOKMARK_ITEM_HEIGHT * (items().size() - 1));
    item->setBounds(-w/2, -BOOKMARK_ITEM_HEIGHT/2, w, BOOKMARK_ITEM_HEIGHT);
    m_itemGroup->addItem(item);
    QGraphicsScene::addItem(item);
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
