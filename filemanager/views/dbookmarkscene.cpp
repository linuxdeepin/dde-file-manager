#include "dbookmarkscene.h"
#include "dbookmarkitem.h"
#include "dbookmarkrootitem.h"
#include "dbookmarkitemgroup.h"
#include "dbookmarkline.h"
#include "windowmanager.h"
#include "dbusinterface/dbustype.h"

#include "../controllers/bookmarkmanager.h"
#include "../controllers/pathmanager.h"

#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"

#include "../deviceinfo/udiskdeviceinfo.h"
#include "../deviceinfo/udisklistener.h"

#include "widgets/singleton.h"

#include <QDebug>
#include <QGraphicsView>
#include <QMimeData>
#include <QDir>

DBookmarkScene::DBookmarkScene(QObject *parent)
    : QGraphicsScene(parent)
{
    initData();
    initUI();
    initConnect();
}

void DBookmarkScene::initData()
{
    QStringList IconKeys;
    IconKeys << "File"
             << "Home"
             << "Desktop"
             << "Videos"
             << "Music"
             << "Pictures"
             << "Documents"
             << "Downloads"
             << "Trash"
             << "Disk"
             << "Usb"
             << "Iphone"
             << "Android"
             << "Network"
             << "Dvd";

    foreach (QString key, IconKeys) {
        QString _key = key.toLower();
        QString smallNormal = QString(":/icons/images/icons/%1_normal_16px.svg").arg(_key);
        QString smallHover = QString(":/icons/images/icons/%1_hover_16px.svg").arg(_key);
        QString smallChecked = QString(":/icons/images/icons/%1_checked_16px.svg").arg(_key);
        QString bigNormal = QString(":/icons/images/icons/%1_normal_22px.svg").arg(_key);
        QString bigHover = QString(":/icons/images/icons/%1_hover_22px.svg").arg(_key);
        QString bigChecked = QString(":/icons/images/icons/%1_checked_22px.svg").arg(_key);
        m_smallIcons.insert(key, smallNormal);
        m_smallHoverIcons.insert(key, smallHover);
        m_smallCheckedIcons.insert(key, smallChecked);
        m_bigIcons.insert(key, bigNormal);
        m_bigHoverIcons.insert(key, bigHover);
        m_bigCheckedIcons.insert(key, bigChecked);
    }


    m_systemPathKeys << "Desktop"
                     << "Videos"
                     << "Music"
                     << "Pictures"
                     << "Documents"
                     << "Downloads";

    m_systemBookMarks["Home"] = tr("Home");
    m_systemBookMarks["Trash"] = tr("Trash");
    m_systemBookMarks["Disk"] = tr("Disk");
    m_systemBookMarks["Network"] = tr("Computers in LAN");

    foreach (QString key, m_systemPathKeys) {
        m_systemBookMarks[key] = systemPathManager->getSystemPathDisplayName(key);
        qDebug() << key << m_systemBookMarks[key];
    }


}

void DBookmarkScene::initUI()
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
}

void DBookmarkScene::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &DBookmarkScene::currentUrlChanged);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkRemove, this, &DBookmarkScene::doBookmarkRemoved);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkAdd, this, &DBookmarkScene::doBookmarkAdded);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkMove, this, &DBookmarkScene::doMoveBookmark);
    connect(fileSignalManager, &FileSignalManager::requestBookmarkRename, this, &DBookmarkScene::bookmarkRename);
    connect(fileSignalManager, &FileSignalManager::bookmarkRenamed, this, &DBookmarkScene::doBookmarkRenamed);

    connect(deviceListener, &UDiskListener::volumeRemoved, this, &DBookmarkScene::volumeRemoved);
    connect(deviceListener, &UDiskListener::mountAdded, this, &DBookmarkScene::mountAdded);
    connect(deviceListener, &UDiskListener::mountRemoved, this, &DBookmarkScene::mountRemoved);

    connect(fileSignalManager, &FileSignalManager::requestChooseSmbMountedFile, this, &DBookmarkScene::chooseMountedItem);
}

DBookmarkItem *DBookmarkScene::createBookmarkByKey(const QString &key)
{
    DBookmarkItem * item = new DBookmarkItem;
    item->boundImageToHover(m_smallIcons.value(key));
    item->boundImageToPress(m_smallCheckedIcons.value(key));
    item->boundImageToRelease(m_smallIcons.value(key));
    item->boundBigImageToHover(m_bigHoverIcons.value(key));
    item->boundBigImageToPress(m_bigCheckedIcons.value(key));
    item->boundBigImageToRelease(m_bigIcons.value(key));
    item->setText(m_systemBookMarks.value(key));
    item->setUrl(getStandardPathByKey(key));
    item->setDefaultItem(true);
    return item;
}


DUrl DBookmarkScene::getStandardPathByKey(const QString &key)
{
    DUrl url;
    if (key == "Recent"){
        url = DUrl::fromRecentFile("/");
    }else if (key == "Home"){
        url = DUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0));
    }else if (key == "Trash"){
        url = DUrl::fromTrashFile("/");
    }else if (key == "Disks"){
        url = DUrl::fromLocalFile("/");
    }else if (m_systemPathKeys.contains(key)){
        url =  DUrl::fromLocalFile(systemPathManager->getSystemPath(key));
    }else{
        qDebug() << "unknown key:" << key;
    }
    return url;
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
    m_defaultLayout->insertItem(index, item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_itemGroup->insert(index, item);
    increaseSize();
    if(item->isDefaultItem())
        m_defaultCount++;
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
    item->deleteLater();
    decreaseSize();
    if(item->isDefaultItem())
        m_defaultCount--;
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
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, SEPARATOR_ITEM_HEIGHT);
    m_itemGroup->addItem(item);
    m_defaultLayout->addItem(item);
    if(item->isDefaultItem())
        m_defaultCount++;
}

void DBookmarkScene::insertSeparator(int index)
{
    DBookmarkLine * item = new DBookmarkLine;
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, SEPARATOR_ITEM_HEIGHT);
    m_itemGroup->insert(index, item);
    m_defaultLayout->insertItem(index, item);
    if(item->isDefaultItem())
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

int DBookmarkScene::getCustomBookmarkItemInsertIndex()
{
    int defaultDiskIndex = indexOf(m_defaultDiskItem);
    return defaultDiskIndex + m_diskItems.count() + 2;
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

void DBookmarkScene::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_Left){
        return;
    }else if (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_Right){
        return;
    }
    QGraphicsScene::keyPressEvent(event);
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
    qDebug() << this << event;
    if(event.windowId() != windowId())
        return;
    if(event.source() == FMEvent::LeftSideBar)
        return;
    m_itemGroup->deselectAll();
    for(int i = 0; i < m_itemGroup->items()->size(); i++)
    {
        DUrl eventUrl = event.fileUrl();
        eventUrl.setQuery("");
        DBookmarkItem* item = m_itemGroup->items()->at(i);
        DUrl itemUrl = item->getUrl();
        itemUrl.setQuery("");

        if(eventUrl == itemUrl)
        {
            item->setChecked(true);
            return;
        }else if (DUrl::childrenList(eventUrl).contains(itemUrl)){
            if (item->isDiskItem()){
                item->setHighlightDisk(true);
                return;
            }
        }
    }
}

void DBookmarkScene::setHomeItem(DBookmarkItem *item)
{
    m_homeItem = item;
}

void DBookmarkScene::setDefaultDiskItem(DBookmarkItem *item)
{
    m_defaultDiskItem = item;
    m_defaultDiskItem->setUrl(DUrl("file:///"));
}

void DBookmarkScene::setNetworkDiskItem(DBookmarkItem *item)
{
    m_networkDiskItem = item;
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
            remove(item);
            bookmarkManager->removeBookmark(item->text(), item->getUrl());

            if (bookmarkManager->getBookmarks().count() == 0){
                DBookmarkLine* lineItem = static_cast<DBookmarkLine*>(m_itemGroup->items()->at(i-1));
                if (lineItem){
                    if (lineItem->objectName() == "DBookmarkLine"){
                        remove(lineItem);
                    }
                }
            }
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

    int insertIndex  = getCustomBookmarkItemInsertIndex();
    insert(insertIndex, item);
    item->setTightMode(m_isTightMode);

    if (bookmarkManager->getBookmarks().size() == 1){
        int networkIndex = indexOf(m_networkDiskItem);
        insertSeparator(networkIndex);
    }


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
        bool isChecked = item->isChecked();
        bool isHighlightDisk = item->isHighlightDisk();
        remove(item);
        m_diskItems.remove(device->getDiskInfo().ID);
        if(isChecked || isHighlightDisk){
            backHome();
        }
        qDebug() << device->getDiskInfo() << item << device->getMountPointUrl();
        emit fileSignalManager->requestAbortJob(device->getMountPointUrl());
        item->deleteLater();
    }
}

void DBookmarkScene::mountAdded(UDiskDeviceInfo *device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().ID);
    if(item)
    {
        item->setDeviceInfo(device);
        item->setMounted(true);
        item->setUrl(device->getMountPointUrl());
    }else{
        QString key = "Disk";
        if (device->getMediaType() == UDiskDeviceInfo::removable){
            key = "Usb";
        }else if (device->getMediaType() == UDiskDeviceInfo::iphone){
            key = "Iphone";
        }else if (device->getMediaType() == UDiskDeviceInfo::phone){
            key = "Android";
        }else if (device->getMediaType() == UDiskDeviceInfo::network){
            key = "Network";
        }else if (device->getMediaType() == UDiskDeviceInfo::camera && device->getName() == "iPhone"){
            key = "Iphone";
        }else if (device->getMediaType() == UDiskDeviceInfo::camera){
            key = "Android";
        }else if (device->getMediaType() == UDiskDeviceInfo::dvd){
            key = "Dvd";
        }
        item = createBookmarkByKey(key);
        item->setDeviceInfo(device);
        item->setUrl(device->getMountPointUrl());

        insert(indexOf(m_defaultDiskItem) + 1 + m_diskItems.count(), item);


        item->setTightMode(m_isTightMode);
        m_diskItems.insert(device->getDiskInfo().ID, item);


        qDebug() << m_delayCheckMountedItem << m_delayCheckMountedEvent;
        if (m_delayCheckMountedItem){
            item->checkMountedItem(m_delayCheckMountedEvent);
        }
        m_delayCheckMountedItem = false;
    }
}

void DBookmarkScene::mountRemoved(UDiskDeviceInfo *device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().ID);
    if(item)
    {
        item->setMounted(false);
        qDebug() << device->getDiskInfo() << item << device->getMountPointUrl();
        emit fileSignalManager->requestAbortJob(device->getMountPointUrl());
        if (item->isChecked() || item->isHighlightDisk()){
            backHome();
        }
        return;
    }
}

void DBookmarkScene::backHome()
{
    FMEvent event;
    event = windowId();
    event = DUrl::fromLocalFile(QDir::homePath());
    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DBookmarkScene::chooseMountedItem(const FMEvent &event)
{
    qDebug() << event << m_diskItems;
    bool checkMountItem = false;
    foreach (QString url, m_diskItems.keys()) {
        if (event.fileUrl() == DUrl(url)){
            m_diskItems.value(url)->checkMountedItem(event);
            checkMountItem = true;
        }
    }

    if (!checkMountItem){
        m_delayCheckMountedEvent = event;
        m_delayCheckMountedItem = true;
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
    bookmarkManager->moveBookmark(indexFrom - getCustomBookmarkItemInsertIndex(), indexTo - getCustomBookmarkItemInsertIndex());
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
    bookmarkManager->moveBookmark(indexFrom - getCustomBookmarkItemInsertIndex(), indexTo - getCustomBookmarkItemInsertIndex());
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
