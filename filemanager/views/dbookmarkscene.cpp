#include "dbookmarkscene.h"
#include "dbookmarkitem.h"
#include "dbookmarkrootitem.h"
#include "dbookmarkitemgroup.h"
#include "../app/global.h"
#include "../controllers/bookmarkmanager.h"
#include <QDebug>
#include <QGraphicsView>
#include <QMimeData>
#include <QDir>
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"
#include "../../deviceinfo/deviceinfo.h"
#include "dbookmarkline.h"

DBookmarkScene::DBookmarkScene()
{
    m_itemGroup = new DBookmarkItemGroup;
    m_rootItem = new DBookmarkRootItem(this);
    m_itemGroup->addItem(m_rootItem);
    QGraphicsScene::addItem(m_rootItem);
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &DBookmarkScene::currentUrlChanged);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkRemove,
            this, &DBookmarkScene::bookmarkRemoved);
    connect(fileSignalManager, &FileSignalManager::deviceAdded, this, &DBookmarkScene::deviceAdded);
    connect(fileSignalManager, &FileSignalManager::deviceRemoved, this, &DBookmarkScene::deviceRemoved);
    connect(fileSignalManager, &FileSignalManager::deviceMounted, this, &DBookmarkScene::bookmarkMounted);
}

void DBookmarkScene::addItem(DBookmarkItem *item)
{
    double w = width();
    item->setPos(w/2, 15 + m_totalHeight);
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
    double dh = m_items.at(index)->boundHeight();
    double dx = m_items.at(index)->x();
    double dy = m_items.at(index)->y() - dh/2;


    for(int i = index; i <  m_items.size(); i++)
    {
        double d1 = m_items.at(i)->x();
        double d2 = m_items.at(i)->y();
        m_items.at(i)->setPos(d1, d2 + BOOKMARK_ITEM_HEIGHT);
    }

    m_items.insert(index, item);
    item->setPos(dx, dy + BOOKMARK_ITEM_HEIGHT/2);
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
    Q_UNUSED(before)
    Q_UNUSED(item)
}

void DBookmarkScene::remove(int index)
{
    Q_UNUSED(index)
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

void DBookmarkScene::addSeparator()
{
    double w = width();
    DBookmarkLine * item = new DBookmarkLine;
    item->setPos(w/2, 3 + m_totalHeight);
    item->setBounds(-w/2, -SEPARATOR_ITEM_HEIGHT/2, w, SEPARATOR_ITEM_HEIGHT);
    m_itemGroup->addItem(item);
    m_items.append(item);
    QGraphicsScene::addItem(item);
    m_totalHeight += SEPARATOR_ITEM_HEIGHT;
    increaseSize();
}

DBookmarkItemGroup *DBookmarkScene::getGroup()
{
    return m_itemGroup;
}

void DBookmarkScene::changed(const QList<QRectF> &region)
{
    Q_UNUSED(region)

    qDebug() << "scene changed";
}

void DBookmarkScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    emit dragEntered();
    QGraphicsScene::dragEnterEvent(event);
    clear(m_rootItem->getDummyItem());

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
        bookmarkManager->removeBookmark(item->text(), item->getUrl());
        remove(item);
    }
}

void DBookmarkScene::currentUrlChanged(const FMEvent &event)
{
//    qDebug() << event.dir << event.source;
    if(event.source() == FMEvent::FileView)
    {
        for(int i = 0; i < m_items.size(); i++)
        {
            if(event.fileUrl() == m_items.at(i)->getUrl())
            {
                m_itemGroup->deselectAll();
                m_items.at(i)->setChecked(true);
                break;
            }
        }
    }
}

void DBookmarkScene::bookmarkRemoved(const QString &url)
{
    for(int i = 0; i < m_items.size(); i++)
    {
        if(url == m_items.at(i)->getUrl())
        {
            remove(m_items.at(i));
            break;
        }
    }
}

void DBookmarkScene::bookmarkMounted(int fd)
{
    qDebug() << "bookmark mounted";
    QStringList mtabMounts;
    QFile mtab("/etc/mtab");
    mtab.open(QFile::ReadOnly);
    QTextStream stream(&mtab);
    do mtabMounts.append(stream.readLine());
    while (!stream.atEnd());
    mtab.close();

    foreach(DBookmarkItem * item, m_items)
    {
        item->setMounted(false);
        item->setReleaseBackgroundColor(Qt::transparent);
    }

    foreach(QString item, mtabMounts)
    {
        QString str = item.split(" ").at(0);
        DBookmarkItem * bookmarkItem = hasBookmarkItem(str);
        if(bookmarkItem && !bookmarkItem->isMounted())
        {
            bookmarkItem->setMounted(true);
            bookmarkItem->setUrl(item.split(" ").at(1));
            bookmarkItem->update();
        }
    }
}

void DBookmarkScene::deviceAdded(DeviceInfo &deviceInfos)
{
    //DBookmarkItem * item = new DBookmarkItem(&deviceInfos);
    //this->insert(11, item);
}

void DBookmarkScene::deviceRemoved(DeviceInfo &deviceInfos)
{
//    QString localPath = deviceInfos.getSysPath();
//    foreach(DBookmarkItem * item, m_items)
//    {
//        if(item->getSysPath() == localPath)
//        {
//            remove(item);
//            return;
//        }
//    }
}

void DBookmarkScene::increaseSize()
{
    if(m_totalHeight > sceneRect().height() - BOOKMARK_ITEM_HEIGHT)
    {
        double w = sceneRect().width();
        setSceneRect(0, 0, w, m_totalHeight + BOOKMARK_ITEM_HEIGHT * 2);
        views().at(0)->setGeometry(0, 0, w, m_totalHeight + BOOKMARK_ITEM_HEIGHT * 2);
    }
}

DBookmarkItem *DBookmarkScene::hasBookmarkItem(const QString &path)
{
    QString localPath = path;
    foreach(DBookmarkItem * item, m_items)
    {
        QString str = localPath.replace("/dev","");
        if(item->getSysPath().contains(str))
        {
            return item;
        }
    }
    return NULL;
}
