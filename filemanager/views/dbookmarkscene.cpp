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
    m_rootItem = new DBookmarkRootItem(this);
    m_defaultLayout = new QGraphicsLinearLayout;
    m_defaultLayout->setOrientation(Qt::Vertical);
    m_defaultLayout->setSpacing(BOOKMARK_ITEM_SPACE);

    QGraphicsScene::addItem(m_rootItem);
    m_rootItem->setContentsMargins(0,0,0,0);
    m_rootItem->setLayout(m_defaultLayout);
    m_rootItem->setPos(0, 0);


    m_itemGroup = new DBookmarkItemGroup;

    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &DBookmarkScene::currentUrlChanged);
    connect(fileSignalManager, &FileSignalManager::deviceAdded, this, &DBookmarkScene::deviceAdded);
    connect(fileSignalManager, &FileSignalManager::deviceRemoved, this, &DBookmarkScene::deviceRemoved);
    connect(fileSignalManager, &FileSignalManager::deviceMounted, this, &DBookmarkScene::bookmarkMounted);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkRemove, this, &DBookmarkScene::doBookmarkRemoved);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkAdd, this, &DBookmarkScene::doBookmarkAdded);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkMove, this, &DBookmarkScene::doMoveBookmark);

}

void DBookmarkScene::addBookmark(DBookmarkItem *item)
{
    m_defaultLayout->addItem(item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_itemGroup->addItem(item);
    increaseSize();
}

void DBookmarkScene::insertBookmark(int index, DBookmarkItem *item)
{
    m_defaultLayout->addItem(item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    m_itemGroup->addItem(item);
    increaseSize();
}

/**
 * @brief DBookmarkScene::addItem
 * @param item
 *
 * Add an item with the given item.
 */
void DBookmarkScene::addItem(DBookmarkItem *item)
{
    m_defaultLayout->addItem(item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    m_itemGroup->addItem(item);
    increaseSize();
}

void DBookmarkScene::addDefaultBookmark(DBookmarkItem *item)
{
    m_defaultLayout->addItem(item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    m_itemGroup->addItem(item);
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
    m_defaultLayout->insertItem(index, item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_itemGroup->addItem(item);
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
 * @brief DBookmarkScene::remove
 * @param item
 *
 * Remove the given item from the item list.
 * Note that the item will be deleted.
 */
void DBookmarkScene::remove(DBookmarkItem *item)
{
    m_defaultLayout->removeItem(item);
    m_itemGroup->removeItem(item);
    delete item;
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
    DBookmarkLine * item = new DBookmarkLine;
    item->setBounds(0, 0, 200, SEPARATOR_ITEM_HEIGHT);
    m_itemGroup->addItem(item);
    m_defaultLayout->addItem(item);
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
    return m_itemGroup->items()->size();
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
    event->accept();
    emit dragEntered();
    QGraphicsScene::dragEnterEvent(event);
    //clear(m_rootItem->getDummyItem());

    if(!event->mimeData()->hasUrls())
        return;
    QList<QUrl> urls = event->mimeData()->urls();
    QUrl firstUrl = urls.at(0);
    QDir dir;
    if(firstUrl.isLocalFile())
        dir.setPath(firstUrl.toLocalFile());
    else
        dir.setPath(firstUrl.toString());
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
    emit dragLeft();
    QGraphicsScene::dragLeaveEvent(event);
}

void DBookmarkScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    QGraphicsScene::dragMoveEvent(event);
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
    qDebug() << "scene drop";
    QGraphicsScene::dropEvent(event);
}

/**
 * @brief DBookmarkScene::doDragFinished
 * @param point
 * @param item
 *
 * The dragged item will be removed when it's not
 * located inside the scene area.
 */
void DBookmarkScene::doDragFinished(const QPointF &point, const QPointF &scenePoint, DBookmarkItem *item)
{
    QGraphicsView * view = this->views().at(0);
    QPoint p(point.x(), point.y());
    QPoint topLeft = view->mapToGlobal(view->geometry().topLeft());
    QPoint bottomRight = view->mapToGlobal(view->geometry().bottomRight());
    QRect rect(topLeft, bottomRight);
    if(!rect.contains(p))
    {    
        FMEvent event;
        event = FMEvent::LeftSideBar;
        event = item->getUrl();
        event = item->windowId();

        emit fileSignalManager->requestBookmarkRemove(event);
    }
    else
    {
        DBookmarkItem * local = itemAt(scenePoint);
        if(local == NULL)
            return;

        if(local->isDefaultItem())
            return;

        int index = indexOf(local);
        if(index == -1)
            return;
        m_defaultLayout->insertItem(index, item);
        int from = indexOf(item);
        bookmarkManager->moveBookmark(from - DEFAULT_ITEM_COUNT, index - DEFAULT_ITEM_COUNT);
        m_itemGroup->items()->move(from, index);
        FMEvent event;

        event = FMEvent::LeftSideBar;
        event = windowId();
        emit fileSignalManager->requestBookmarkMove(from, index, event);
    }
}

void DBookmarkScene::currentUrlChanged(const FMEvent &event)
{
    if(event.windowId() != windowId())
        return;
    if(event.source() == FMEvent::FileView)
    {
        m_itemGroup->deselectAll();
        for(int i = 0; i < m_itemGroup->items()->size(); i++)
        {
            if(event.fileUrl() == m_itemGroup->items()->at(i)->getUrl())
            {
                m_itemGroup->items()->at(i)->setChecked(true);
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
    for(int i = 0; i < m_itemGroup->items()->size(); i++)
    {
        if(event.fileUrl() == m_itemGroup->items()->at(i)->getUrl())
        {
            DBookmarkItem * item = m_itemGroup->items()->at(i);
            bookmarkManager->removeBookmark(item->text(), item->getUrl());
            remove(item);
            break;
        }
    }
}

void DBookmarkScene::doBookmarkAdded(const QString &name, const FMEvent &event)
{
    DBookmarkItem * item = DBookmarkItem::makeBookmark(name, event.fileUrl());
    item->setPos(0, m_defaultLayout->count() * 30);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    insert(DEFAULT_ITEM_COUNT, item);
}

void DBookmarkScene::doMoveBookmark(int from, int to, const FMEvent &event)
{
    if(windowId() != event.windowId())
    {
        qDebug() << m_itemGroup->items()->size();
        m_defaultLayout->insertItem(to,  m_itemGroup->items()->at(from));
        m_itemGroup->items()->move(from, to);
    }
}

void DBookmarkScene::rootDropped(const QPointF &point)
{

}

void DBookmarkScene::increaseSize()
{
    QRectF rect = sceneRect();
    if(count() * 30 > rect.height())
    {
        setSceneRect(rect.x(), rect.y(), rect.width(), rect.height() + 30);
    }
}

void DBookmarkScene::decreaseSize()
{
    QRectF rect = sceneRect();
    if(count() * 30 < rect.height())
    {
        setSceneRect(rect.x(), rect.y(), rect.width(), rect.height() - 30);
    }
}

DBookmarkItem *DBookmarkScene::hasBookmarkItem(const DUrl &url)
{
    QList<DBookmarkItem *> list = *m_itemGroup->items();
    foreach(DBookmarkItem * item, list)
    {
        if(item->getUrl() == url)
        {
            return item;
        }
    }
    return NULL;
}

DBookmarkItem *DBookmarkScene::itemAt(const QPointF & point)
{
    return (DBookmarkItem *)QGraphicsScene::itemAt(point + QPointF(0, 15), QTransform());
}

int DBookmarkScene::indexOf(DBookmarkItem *item)
{
    return m_itemGroup->items()->indexOf(item);
}

void DBookmarkScene::setTightMode(bool v)
{
    for(int i = 0; i < m_itemGroup->items()->size(); i++)
    {
        m_itemGroup->items()->at(i)->setTightMode(v);
    }
    update();
}
