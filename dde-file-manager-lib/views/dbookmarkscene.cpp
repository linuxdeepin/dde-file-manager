#include "dbookmarkscene.h"
#include "dbookmarkitem.h"
#include "dbookmarkrootitem.h"
#include "dbookmarkitemgroup.h"
#include "dbookmarkline.h"
#include "windowmanager.h"
#include "dfmeventdispatcher.h"
#include "dbusinterface/dbustype.h"
#include "dfilemenu.h"

#include "controllers/bookmarkmanager.h"
#include "controllers/pathmanager.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "deviceinfo/udisklistener.h"
#include "dialogs/dialogmanager.h"

#include "singleton.h"
#include <ddialog.h>

#include <QDebug>
#include <QGraphicsView>
#include <QMimeData>
#include <QDir>

DWIDGET_USE_NAMESPACE
DFM_USE_NAMESPACE

DBookmarkScene::DBookmarkScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_delayCheckMountedEvent(this)
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
             << "Computer"
             << "Disk"
             << "Usb"
             << "Iphone"
             << "Android"
             << "Network"
             << "UserShare"
             << "Dvd"
             << "BookMarks"
             << "System Disk";

    foreach (QString key, IconKeys) {
        QString _key = key.toLower();

        /* specially handle for system disk icon(disk icon)*/
        if(key == "System Disk")
            _key = "disk";
        QString smallNormal = QString(":/leftsidebar/images/leftsidebar/%1_normal_16px.svg").arg(_key);
        QString smallHover = QString(":/leftsidebar/images/leftsidebar/%1_active_16px.svg").arg(_key);
        QString smallChecked = QString(":/leftsidebar/images/leftsidebar/%1_active_16px.svg").arg(_key);
        m_smallIcons.insert(key, smallNormal);
        m_smallHoverIcons.insert(key, smallHover);
        m_smallCheckedIcons.insert(key, smallChecked);
    }


    m_systemPathKeys << "Home"
                     << "Desktop"
                     << "Videos"
                     << "Music"
                     << "Pictures"
                     << "Documents"
                     << "Downloads"
                     << "Trash"
                     << "Computer"
                     << "System Disk"
                     << "Network"
                     << "UserShare";

    foreach (QString key, m_systemPathKeys) {
        m_systemBookMarks[key] = systemPathManager->getSystemPathDisplayName(key);
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

    connect(deviceListener, &UDiskListener::volumeAdded, this, &DBookmarkScene::volumeAdded);
    connect(deviceListener, &UDiskListener::volumeRemoved, this, &DBookmarkScene::volumeRemoved);
    connect(deviceListener, &UDiskListener::volumeChanged, this, &DBookmarkScene::volumeChanged);
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
    item->setText(m_systemBookMarks.value(key));
    item->setUrl(getStandardPathByKey(key));
    item->setDefaultItem(true);
    return item;
}

DBookmarkItem *DBookmarkScene::createCustomBookmark(const QString &name, const DUrl &url)
{
    QString key = "BookMarks";
    DBookmarkItem * item = new DBookmarkItem;
    item->boundImageToHover(m_smallIcons.value(key));
    item->boundImageToPress(m_smallCheckedIcons.value(key));
    item->boundImageToRelease(m_smallIcons.value(key));
    item->setText(name);
    item->setUrl(url);
    item->setDefaultItem(false);
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
    }else if (key == "Computer"){
        url = DUrl::fromComputerFile("/");
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
    if (m_disableUrlSchemeList.contains(item->getUrl().scheme())) {
        item->hide();
    }

    m_defaultLayout->addItem(item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_itemGroup->addItem(item);
    updateSceneRect();
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
    if (m_disableUrlSchemeList.contains(item->getUrl().scheme())) {
        item->hide();
    }

    m_defaultLayout->insertItem(index, item);
    item->setBounds(0, 0, BOOKMARK_ITEM_WIDTH, BOOKMARK_ITEM_HEIGHT - BOOKMARK_ITEM_SPACE);
    connect(item, &DBookmarkItem::dragFinished, this, &DBookmarkScene::doDragFinished);
    m_itemGroup->insert(index, item);
    updateSceneRect();
    if(item->isDefaultItem())
        m_defaultCount++;
}

void DBookmarkScene::insert(DBookmarkItem *before, DBookmarkItem *item)
{
    if (m_disableUrlSchemeList.contains(item->getUrl().scheme())) {
        item->hide();
    }

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
    removeItem(item);
    item->deleteLater();
    updateSceneRect();
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
    return m_itemGroup->items().size();
}

int DBookmarkScene::getCustomBookmarkItemInsertIndex()
{
    int defaultDiskIndex = indexOf(m_defaultDiskItem);
    return defaultDiskIndex + m_diskItems.count() + 2;
}

int DBookmarkScene::windowId()
{
    return WindowManager::getWindowId(views().at(0));
}

/**
 * @brief DBookmarkScene::dragEnterEvent
 * @param event
 *
 * A dummy item will be temporarily added
 * if a drag carrying valid url has occurred.
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
 * as drop event has occurred.
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
        DFMUrlBaseEvent event(this, item->getUrl());

        event.setWindowId(item->windowId());

        emit fileSignalManager->requestBookmarkRemove(event);
    }
    else
    {
        DBookmarkItem * local = itemAt(scenePoint);
        if(local == NULL)
        {
            local = m_itemGroup->items().last();
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

void DBookmarkScene::currentUrlChanged(const DFMEvent &event)
{
    qDebug() << this << event;
    if(event.windowId() != windowId())
        return;
    if (event.sender() == this)
        return;
    setCurrentUrl(event.fileUrl());
}


void DBookmarkScene::setCurrentUrl(DUrl url)
{
    m_itemGroup->deselectAll();
    url.setQuery("");
    for(int i = 0; i < m_itemGroup->items().size(); i++)
    {
        DBookmarkItem* item = m_itemGroup->items().at(i);
        DUrl itemUrl = item->getUrl();
        itemUrl.setQuery("");

        if(url == itemUrl)
        {
            item->setChecked(true);
            return;
        }/*else if (DUrl::childrenList(url).contains(itemUrl)){
            if (item->isDiskItem() && ){
                item->setHighlightDisk(true);
                return;
            }
        }*/
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
void DBookmarkScene::doBookmarkRemoved(const DFMEvent &event)
{
    DBookmarkItem * item = hasBookmarkItem(event.fileUrl());
    if(!item)
        return;
    int item_index = indexOf(item);
    remove(item);
    bookmarkManager->removeBookmark(item->getBookmarkModel());
    if (bookmarkManager->getBookmarks().count() == 0){
        DBookmarkLine* lineItem = qobject_cast<DBookmarkLine*>(m_itemGroup->items().value(item_index - 1));

        if (lineItem && lineItem->objectName() == "DBookmarkLine") {
            remove(lineItem);
        }
    }
}

void DBookmarkScene::bookmarkRename(const DFMEvent &event)
{
    if(windowId() != event.windowId())
        return;

    DBookmarkItem* item = hasBookmarkItem(event.fileUrl());

    if (item)
        item->editMode();
}

void DBookmarkScene::doBookmarkRenamed(const QString &newname,const DFMEvent &event)
{
    DBookmarkItem * item = hasBookmarkItem(event.fileUrl());
    if(item)
        item->setText(newname);
}

void DBookmarkScene::doBookmarkAdded(const QString &name, const DFMEvent &event)
{
    DBookmarkItem * item = createCustomBookmark(name, event.fileUrl());
    item->setIsCustomBookmark(true);
    item->setBookmarkModel(bookmarkManager->getBookmarks().at(0));

    int insertIndex  = getCustomBookmarkItemInsertIndex();
    insert(insertIndex, item);
    item->setTightMode(m_isTightMode);

    if (bookmarkManager->getBookmarks().size() == 1){
        int networkIndex = indexOf(m_networkDiskItem);
        insertSeparator(networkIndex);
    }


}

void DBookmarkScene::doMoveBookmark(int from, int to, const DFMEvent &event)
{
    if(windowId() != event.windowId())
    {
        qDebug() << m_itemGroup->items().size();
        m_defaultLayout->insertItem(to,  m_itemGroup->items().at(from));
        m_itemGroup->items().move(from, to);
    }
}


void DBookmarkScene::volumeAdded(UDiskDeviceInfoPointer device)
{
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
    DBookmarkItem * item = createBookmarkByKey(key);
    item->setDeviceInfo(device);

    insert(indexOf(m_defaultDiskItem) + 1 + m_diskItems.count(), item);

    item->setTightMode(m_isTightMode);
    m_diskItems.insert(device->getDiskInfo().id(), item);

}

void DBookmarkScene::volumeRemoved(UDiskDeviceInfoPointer device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().id());
    if(item)
    {
        handleVolumeMountRemove(device, item);
        m_diskItems.remove(device->getDiskInfo().id());
        remove(item);
        item->deleteLater();
    }
}

void DBookmarkScene::volumeChanged(UDiskDeviceInfoPointer device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().id());
    if(item)
    {
        item->setDeviceInfo(device);
        item->update();
        bool isChecked = item->isChecked();
        bool isHighlightDisk = item->isHighlightDisk();
        if(isChecked || isHighlightDisk){
            DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, device->getMountPointUrl(), views().at(0)->window());
            emit fileSignalManager->requestFreshFileView(windowId());
        }
    }
}

void DBookmarkScene::mountAdded(UDiskDeviceInfoPointer device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().id());
    if(item)
    {
        item->setDeviceInfo(device);
        item->setMounted(true);
        item->setUrl(device->getMountPointUrl());
    }else{
        volumeAdded(device);
        item = m_diskItems.value(device->getDiskInfo().id());
    }
    qDebug() << m_delayCheckMountedItem << m_delayCheckMountedEvent;
    if (m_delayCheckMountedItem && !item->getMountBookmark()){
        item->checkMountedItem(m_delayCheckMountedEvent);
        m_delayCheckMountedItem = false;
        return;
    }

    for(int i = 0; i < m_itemGroup->items().size(); i++)
    {
        DBookmarkItem* item = m_itemGroup->items().at(i);
        if (item->getIsCustomBookmark() && item->getMountBookmark()){
             DUrlList urls;
             urls << item->getUrl();
             DFMEventDispatcher::instance()->processEventAsync<DFMOpenUrlEvent>(item, urls, DFMOpenUrlEvent::OpenInCurrentWindow);
             item->setMountBookmark(false);
        }
    }
}

void DBookmarkScene::mountRemoved(UDiskDeviceInfoPointer device)
{
    DBookmarkItem * item = m_diskItems.value(device->getDiskInfo().id());
    if(item)
    {
        if (device->getDiskInfo().has_volume()){
            item->setDeviceInfo(device);
            item->setMounted(false);
            handleVolumeMountRemove(device, item);
        }else{
            volumeRemoved(device);
        }
        return;
    }
}

void DBookmarkScene::handleVolumeMountRemove(UDiskDeviceInfoPointer device, DBookmarkItem *item)
{
    bool isChecked = item->isChecked();
    bool isHighlightDisk = item->isHighlightDisk();
    if(isChecked || isHighlightDisk){
        backHome();
    }
    emit fileSignalManager->requestAbortJob(device->getMountPointUrl());
}

void DBookmarkScene::backHome()
{
    DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, DUrl::fromLocalFile(QDir::homePath()), views().at(0)->window());
}

void DBookmarkScene::chooseMountedItem(const DFMEvent &event)
{
    qDebug() << event << m_diskItems;
    bool checkMountItem = false;
    foreach (QString url, m_diskItems.keys()) {
        DUrl url1 = event.fileUrl();
        DUrl url2(url);

        if (url1.path().isEmpty()){
            url1.setPath("/");
        }

        if (url2.path().isEmpty()){
            url2.setPath("/");
        }

        if (url1 == url2){
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
    DBookmarkItem *item = m_itemGroup->items().last();
    qDebug() << item->geometry().bottomLeft().y() << point.y();
    if(!item->isDefaultItem() && item->geometry().bottomLeft().y() < point.y())
        return true;
    else
        return false;
}

void DBookmarkScene::updateSceneRect()
{
    int height = 0;
    QRectF rect = sceneRect();
    foreach (DBookmarkItem* item, m_itemGroup->items()) {
        height += item->boundHeight();
    }
    setSceneRect(10, 10, rect.width(), height);
    emit sceneRectChanged();
}

void DBookmarkScene::moveBefore(DBookmarkItem *from, DBookmarkItem *to)
{
    int indexFrom = m_itemGroup->items().indexOf(from);
    int indexTo = m_itemGroup->items().indexOf(to);

    if(indexFrom == -1 || indexTo == -1)
        return;

    if(indexFrom < indexTo)
    {
        indexTo -= 1;
    }

    m_defaultLayout->insertItem(indexTo, from);
    bookmarkManager->moveBookmark(indexFrom - getCustomBookmarkItemInsertIndex(), indexTo - getCustomBookmarkItemInsertIndex());
    m_itemGroup->items().move(indexFrom, indexTo);

    DFMEvent event(this);
    event.setWindowId(windowId());
    emit fileSignalManager->requestBookmarkMove(indexFrom, indexTo, event);
}

void DBookmarkScene::moveAfter(DBookmarkItem *from, DBookmarkItem *to)
{
    int indexFrom = m_itemGroup->items().indexOf(from);
    int indexTo = m_itemGroup->items().indexOf(to);

    if(indexFrom == -1 || indexTo == -1)
        return;

    m_defaultLayout->insertItem(indexTo, from);
    bookmarkManager->moveBookmark(indexFrom - getCustomBookmarkItemInsertIndex(), indexTo - getCustomBookmarkItemInsertIndex());
    m_itemGroup->items().move(indexFrom, indexTo);

    DFMEvent event(this);
    event.setWindowId(windowId());
    emit fileSignalManager->requestBookmarkMove(indexFrom, indexTo, event);
}

DBookmarkItem *DBookmarkScene::hasBookmarkItem(const DUrl &url)
{
    foreach(DBookmarkItem * item, m_itemGroup->items())
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
    if(item){
        if(item->objectName() == "BMRootItem")
            return NULL;
        return item;
    }
    else
        return (DBookmarkItem *)QGraphicsScene::itemAt(point + QPointF(0, BOOKMARK_ITEM_SPACE*2), QTransform());
}

int DBookmarkScene::indexOf(DBookmarkItem *item)
{
    return m_itemGroup->items().indexOf(item);
}

void DBookmarkScene::setTightMode(bool v)
{
    for(int i = 0; i < m_itemGroup->items().size(); i++)
    {
        m_itemGroup->items().at(i)->setTightMode(v);
    }
    m_isTightMode = v;
    update();
}

void DBookmarkScene::setDisableUrlSchemes(const QList<QString> &schemes)
{
    for (DBookmarkItem *item : m_itemGroup->items()) {
        const QString &scheme = item->getUrl().scheme();

        if (schemes.contains(scheme)){
            item->hide();
        }
    }
    m_disableUrlSchemeList = schemes;
}
