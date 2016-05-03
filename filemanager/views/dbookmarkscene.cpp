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
#include "windowmanager.h"

DBookmarkScene::DBookmarkScene()
{
    m_acceptDrop = true;
    m_totalHeight = 0;
    m_itemGroup = new DBookmarkItemGroup;
    m_rootItem = new DBookmarkRootItem(this);
    m_itemGroup->addItem(m_rootItem);
    QGraphicsScene::addItem(m_rootItem);
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &DBookmarkScene::currentUrlChanged);
    connect(fileSignalManager, &FileSignalManager::deviceAdded, this, &DBookmarkScene::deviceAdded);
    connect(fileSignalManager, &FileSignalManager::deviceRemoved, this, &DBookmarkScene::deviceRemoved);
    connect(fileSignalManager, &FileSignalManager::deviceMounted, this, &DBookmarkScene::bookmarkMounted);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkRemove, this, &DBookmarkScene::doBookmarkRemoved);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkAdd, this, &DBookmarkScene::doBookmarkAdded);
}

void DBookmarkScene::addBookmark(DBookmarkItem *item)
{
    addItem(item);
    m_customItems.append(item);
}

void DBookmarkScene::insertBookmark(int index, DBookmarkItem *item)
{
    if(m_customItems.size() == 0)
    {
        addItem(item);
        m_customItems.append(item);
    }
    else
    {
        int len = m_items.size() - m_customItems.size();
        insert(len + index, item);
        m_customItems.insert(index, item);
    }
}

/**
 * @brief DBookmarkScene::addItem
 * @param item
 *
 * Add an item with the given item.
 */
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

/**
 * @brief DBookmarkScene::insert
 * @param index
 * @param item
 *
 * Insert an item before the item of the given index.
 * If the given index is the last one, the item will
 * be added as the last item. The function will do
 * nothing if the given index is greater than the size
 * of the list of added items.
 */
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

/**
 * @brief DBookmarkScene::clear
 * @param item
 *
 * Clear the given item from the item list.
 * Note that the given item will not be deleted.
 */
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

/**
 * @brief DBookmarkScene::remove
 * @param item
 *
 * Remove the given item from the item list.
 * Note that the item will be deleted.
 */
void DBookmarkScene::remove(DBookmarkItem *item)
{
    int i = m_items.indexOf(item);
    if(i < 0)
        return;
    double dh = item->boundHeight();
    for(int index = i; index < m_items.size(); index++)
    {
        double d1 = m_items.at(index)->x();
        double d2 = m_items.at(index)->y();
        m_items.at(index)->setPos(d1, d2 - dh);
    }
    m_items.removeOne(item);
    m_customItems.removeOne(item);
    m_itemGroup->removeItem(item);
    QGraphicsScene::removeItem(item);
    item->deleteLater();
    m_totalHeight -= dh;
    decreaseSize();
}

void DBookmarkScene::setSceneRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rootItem->setBounds(x, y, w, h);
    QGraphicsScene::setSceneRect(x, y, w, h);
}


/**
 * @brief DBookmarkScene::addSeparator
 *
 * Add a separator line after the last item
 * of the item list.
 */
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

/**
 * @brief DBookmarkScene::getGroup
 * @return
 *
 * Return the group of all the added items.
 */
DBookmarkItemGroup *DBookmarkScene::getGroup()
{
    return m_itemGroup;
}

/**
 * @brief DBookmarkScene::count
 * @return
 *
 * Return the size of the item list.
 */
int DBookmarkScene::count()
{
    return m_items.size();
}

int DBookmarkScene::windowId()
{
    return WindowManager::getWindowId(views().at(0)->window());
}

/**
 * @brief DBookmarkScene::dragEnterEvent
 * @param event
 *
 * A dummy item will be temporarily added
 * if a drag carrying valid url has occured.
 * Note that a separator line will be added
 * if no custom bookmark exists in the list.
 */
void DBookmarkScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if(!m_acceptDrop)
        return;
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
        qDebug() << count();
        if(count() > DEFAULT_ITEM_COUNT)
            insert(DEFAULT_ITEM_COUNT, m_rootItem->getDummyItem());
        else
            insert(DEFAULT_ITEM_COUNT - 1, m_rootItem->getDummyItem());
    }
}

/**
 * @brief DBookmarkScene::dragLeaveEvent
 * @param event
 *
 * The dummy item will be cleared(not be deleted)
 * as the drag has left out of the scene area.
 */
void DBookmarkScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    if(!m_acceptDrop)
        return;
    emit dragLeft();
    QGraphicsScene::dragLeaveEvent(event);
    clear(m_rootItem->getDummyItem());
}

/**
 * @brief DBookmarkScene::dropEvent
 * @param event
 *
 * The dummy item will be cleared(not be deleted)
 * as drop event has occured.
 */
void DBookmarkScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dropEvent(event);
    clear(m_rootItem->getDummyItem());
}

/**
 * @brief DBookmarkScene::doDragFinished
 * @param point
 * @param item
 *
 * The dragged item will be removed when it's not
 * located inside the scene area.
 */
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
        FMEvent event;
        event = FMEvent::LeftSideBar;
        event = item->getUrl();
        event = item->windowId();

        emit fileSignalManager->requestBookmarkRemove(event);
        remove(item);
        if(count() == DEFAULT_ITEM_COUNT)
            remove(m_items.last());
    }
}

void DBookmarkScene::currentUrlChanged(const FMEvent &event)
{
    if(event.windowId() != windowId())
        return;
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
    Q_UNUSED(url)
}

void DBookmarkScene::bookmarkMounted(int fd)
{
    Q_UNUSED(fd);
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
        DBookmarkItem * bookmarkItem = hasBookmarkItem(DUrl(str));
        if(bookmarkItem && !bookmarkItem->isMounted())
        {
            bookmarkItem->setMounted(true);
            bookmarkItem->setUrl(DUrl(item.split(" ").at(1)));
            bookmarkItem->update();
        }
    }
}

void DBookmarkScene::deviceAdded(DeviceInfo &deviceInfos)
{
    Q_UNUSED(deviceInfos);
    //DBookmarkItem * item = new DBookmarkItem(&deviceInfos);
    //this->insert(11, item);
}

void DBookmarkScene::deviceRemoved(DeviceInfo &deviceInfos)
{
    Q_UNUSED(deviceInfos);
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


/**
 * @brief DBookmarkScene::bookmarkRemoved
 * @param url
 *
 * Remove the custom bookmark with the given url.
 */
void DBookmarkScene::doBookmarkRemoved(const FMEvent &event)
{
    for(int i = 0; i < m_items.size(); i++)
    {
        if(event.fileUrl() == m_items.at(i)->getUrl())
        {
            DBookmarkItem * item = m_items.at(i);
            bookmarkManager->removeBookmark(item->text(), item->getUrl());
            remove(item);

            if(count() == DEFAULT_ITEM_COUNT)
                remove(m_items.last());
            break;
        }
    }
}

void DBookmarkScene::doBookmarkAdded(const QString &name, const FMEvent &event)
{
    DBookmarkItem * item = DBookmarkItem::makeBookmark(name, event.fileUrl());

    if(count() == DEFAULT_ITEM_COUNT - 1)
        addSeparator();
    insert(DEFAULT_ITEM_COUNT, item);
}

void DBookmarkScene::increaseSize()
{
    if(m_totalHeight > sceneRect().height() - BOOKMARK_ITEM_HEIGHT)
    {
        double w = sceneRect().width();
        setSceneRect(0, 0, w, m_totalHeight + BOOKMARK_ITEM_HEIGHT * 2);
        views().at(0)->resize(w, m_totalHeight + BOOKMARK_ITEM_HEIGHT * 2);
    }
}

void DBookmarkScene::decreaseSize()
{
    if(m_totalHeight < sceneRect().height() - BOOKMARK_ITEM_HEIGHT)
    {
        double w = sceneRect().width();
        views().at(0)->resize(w, m_totalHeight + BOOKMARK_ITEM_HEIGHT * 2);
        setSceneRect(0, 0, w, m_totalHeight + BOOKMARK_ITEM_HEIGHT);
    }
}

DBookmarkItem *DBookmarkScene::hasBookmarkItem(const DUrl &url)
{
    foreach(DBookmarkItem * item, m_items)
    {
        if(item->getUrl() == url)
        {
            return item;
        }
    }
    return NULL;
}

void DBookmarkScene::setAcceptDrop(bool v)
{
    m_acceptDrop = v;
    m_rootItem->setAcceptDrops(false);
}
