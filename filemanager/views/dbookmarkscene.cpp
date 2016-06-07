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
#include "../../deviceinfo/udiskdeviceinfo.h"
#include "dbookmarkline.h"
#include "windowmanager.h"
#include "dbusinterface/dbustype.h"

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
    connect(fileSignalManager, &FileSignalManager::requestBookmarkRemove, this, &DBookmarkScene::doBookmarkRemoved);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkAdd, this, &DBookmarkScene::doBookmarkAdded);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkMove, this, &DBookmarkScene::doMoveBookmark);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkRename, this, &DBookmarkScene::bookmarkRename);
    connect(fileSignalManager, &FileSignalManager::bookmarkRenamed, this, &DBookmarkScene::doBookmarkRenamed);
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
    item->setBounds(-1, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_itemGroup->addItem(item);
    increaseSize();
    if(item->isDefaultItem())
        m_defaultCount++;
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
    if(item->isDefaultItem())
        m_defaultCount++;
    m_defaultLayout->insertItem(index, item);
    item->setBounds(-1, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_itemGroup->insert(index, item);
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
    if(item->isDefaultItem())
        m_defaultCount--;
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
    item->setBounds(-1, 0, 200, SEPARATOR_ITEM_HEIGHT);
    m_itemGroup->addItem(item);
    m_defaultLayout->addItem(item);
    m_defaultCount++;
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
    emit dragLeft();
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
        {
            local = m_itemGroup->items()->last();
            if(isBelowLastItem(scenePoint))
                moveAfter(item, local);
            return;
        }
        if(local->isDefaultItem())
            return;
        moveBefore(item, local);
    }
    emit dragLeft();
}

void DBookmarkScene::currentUrlChanged(const FMEvent &event)
{
    if(event.windowId() != windowId())
        return;
    if(event.source() == FMEvent::LeftSideBar)
        return;
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

void DBookmarkScene::bookmarkRename(const FMEvent &event)
{
    if(windowId() != event.windowId())
        return;

    for(int i = 0; i < m_itemGroup->items()->size(); i++)
    {
        if(event.fileUrl() == m_itemGroup->items()->at(i)->getUrl())
        {
            DBookmarkItem * item = m_itemGroup->items()->at(i);
            item->editMode();
            break;
        }
    }
}

void DBookmarkScene::doBookmarkRenamed(const QString &oldname, const QString &newname,const FMEvent &event)
{
    for(int i = 0; i < m_itemGroup->items()->size(); i++)
    {
        DBookmarkItem * item = m_itemGroup->items()->at(i);
        if(event.fileUrl() == item->getUrl() && item->text() == oldname)
        {
            DBookmarkItem * item = m_itemGroup->items()->at(i);
            item->setText(newname);
            break;
        }
    }
}

void DBookmarkScene::doBookmarkAdded(const QString &name, const FMEvent &event)
{
    DBookmarkItem * item = DBookmarkItem::makeBookmark(name, event.fileUrl());
    insert(m_defaultCount, item);
    item->setTightMode(m_isTightMode);
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

void DBookmarkScene::volumeAdded(UDiskDeviceInfo *device)
{
    Q_UNUSED(device);
}

void DBookmarkScene::volumeRemoved(UDiskDeviceInfo *device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().ID);
    if(item)
    {
        remove(item);
        m_diskItems.remove(device->getDiskInfo().ID);
    }
}

void DBookmarkScene::mountAdded(UDiskDeviceInfo *device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().ID);
    qDebug() << item;
    if(item)
    {
        item->setDeviceInfo(device);
        item->setMounted(true);
    }else{
        item = new DBookmarkItem(device);
        insert(m_defaultCount -1, item);
        item->setTightMode(m_isTightMode);
        m_diskItems.insert(device->getDiskInfo().ID, item);
    }
}

void DBookmarkScene::mountRemoved(UDiskDeviceInfo *device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().ID);
    if(item)
    {
        item->setMounted(false);
        return;
    }
}

bool DBookmarkScene::isBelowLastItem(const QPointF &point)
{
    DBookmarkItem *item = m_itemGroup->items()->last();
    qDebug() << item->geometry().bottomLeft().y() << point.y();
    if(!item->isDefaultItem() && item->geometry().bottomLeft().y() < point.y())
        return true;
    else
        return false;
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

void DBookmarkScene::moveBefore(DBookmarkItem *from, DBookmarkItem *to)
{
    int indexFrom = m_itemGroup->items()->indexOf(from);
    int indexTo = m_itemGroup->items()->indexOf(to);

    if(indexFrom == -1 || indexTo == -1)
        return;

    if(indexFrom < indexTo)
    {
        indexTo -= 1;
    }

    m_defaultLayout->insertItem(indexTo, from);
    bookmarkManager->moveBookmark(indexFrom - m_defaultCount, indexTo - m_defaultCount);
    m_itemGroup->items()->move(indexFrom, indexTo);

    FMEvent event;
    event = FMEvent::LeftSideBar;
    event = windowId();
    emit fileSignalManager->requestBookmarkMove(indexFrom, indexTo, event);
}

void DBookmarkScene::moveAfter(DBookmarkItem *from, DBookmarkItem *to)
{
    int indexFrom = m_itemGroup->items()->indexOf(from);
    int indexTo = m_itemGroup->items()->indexOf(to);

    if(indexFrom == -1 || indexTo == -1)
        return;

    m_defaultLayout->insertItem(indexTo, from);
    bookmarkManager->moveBookmark(indexFrom - m_defaultCount, indexTo - m_defaultCount);
    m_itemGroup->items()->move(indexFrom, indexTo);

    FMEvent event;
    event = FMEvent::LeftSideBar;
    event = windowId();
    emit fileSignalManager->requestBookmarkMove(indexFrom, indexTo, event);
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
    DBookmarkItem * item =  (DBookmarkItem *)QGraphicsScene::itemAt(point, QTransform());
    if(item->objectName() == "BMRootItem")
        return NULL;
    if(item)
        return item;
    else
        return (DBookmarkItem *)QGraphicsScene::itemAt(point + QPointF(0, BOOKMARK_ITEM_SPACE*2), QTransform());
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
    m_isTightMode = v;
    update();
}
