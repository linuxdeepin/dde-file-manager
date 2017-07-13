#include "dleftsidebar.h"
#include "dcheckablebutton.h"
#include "dhorizseparator.h"
#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include "dfmeventdispatcher.h"

#include "controllers/bookmarkmanager.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "usershare/usersharemanager.h"

#include "deviceinfo/udisklistener.h"

#include "singleton.h"

#include "dhoverbutton.h"
#include "dbookmarkitem.h"
#include "dbookmarkscene.h"
#include "dbookmarkitemgroup.h"
#include "dbookmarkrootitem.h"
#include "dfileview.h"
#include "dtoolbar.h"
#include "plugins/pluginmanager.h"
#include "view/viewinterface.h"

#include <QVBoxLayout>
#include <QButtonGroup>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QListWidgetItem>
#include <QLabel>


DLeftSideBar::DLeftSideBar(QWidget *parent) : QFrame(parent)
{
    initData();
    initUI();
    initConnect();
}

DLeftSideBar::~DLeftSideBar()
{

}

void DLeftSideBar::initData()
{
    m_nameList << "Home"
               << "Desktop"
               << "Videos"
               << "Music"
               << "Pictures"
               << "Documents"
               << "Downloads"
               << "Trash"
               << "Separator"
               << "Computer"
               << "System Disk";

    m_navState = true;
    setAcceptDrops(true);
}

void DLeftSideBar::initUI()
{
    setFocusPolicy(Qt::NoFocus);
    initNav();

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_nav);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    loadBookmark();
    addNetworkBookmarkItem();
    addUserShareBookmarkItem();
    loadDevices();
    loadPluginBookmarks();

    m_view->centerOn(0,0);
}

void DLeftSideBar::initConnect()
{
//    connect(m_fileButton, &DCheckableButton::released, this, &DLeftSideBar::navSwitched);
    connect(m_scene->getGroup(), &DBookmarkItemGroup::url, this, &DLeftSideBar::handleLocationChanged);
    connect(m_scene, &DBookmarkScene::dragEntered, this, &DLeftSideBar::doDragEnter);
    connect(m_scene, &DBookmarkScene::dragLeft, this, &DLeftSideBar::doDragLeave);
    connect(m_scene, &DBookmarkScene::dropped, this, &DLeftSideBar::doDragLeave);
//    connect(deviceListener, &UDiskListener::requestDiskInfosFinihsed, this, &DLeftSideBar::handdleRequestDiskInfosFinihsed);
    connect(fileSignalManager, &FileSignalManager::userShareCountChanged, this, &DLeftSideBar::handleUserShareCountChanged);
    connect(userShareManager, &UserShareManager::userShareAdded, this, &DLeftSideBar::centerOnMyShareItem);
    connect(m_scene, &DBookmarkScene::sceneRectChanged, this, &DLeftSideBar::updateVerticalScrollBar);
}

void DLeftSideBar::initNav()
{
    m_nav = new QFrame;
    m_nav->setStyleSheet("QFrame{border:0px solid red}");
    m_nav->setFocusPolicy(Qt::NoFocus);
    m_nav->setFixedWidth(200);
    QVBoxLayout* navLayout = new QVBoxLayout;

    m_view = new QGraphicsView;
    m_view->setAcceptDrops(true);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setObjectName("Bookmark");
    m_view->setAlignment(Qt::AlignTop);
    m_scene = new DBookmarkScene(this);
//    m_scene->setSceneRect(10, 10, 200, 200);
    m_view->setScene(m_scene);

    m_verticalScrollBar = m_view->verticalScrollBar();
    m_verticalScrollBar->setParent(this);
    m_verticalScrollBar->setObjectName("LeftsideBar");

    foreach (QString key, m_nameList) {
        if (key == "Separator"){
            m_scene->addSeparator();
        }else{
            DBookmarkItem * item = m_scene->createBookmarkByKey(key);
            m_scene->addItem(item);
            if (key == "Home"){
                m_scene->setHomeItem(item);
            }
            if (key == "System Disk"){
                m_scene->setDefaultDiskItem(item);
            }
        }
    }
//    navLayout->addWidget(fileFrame);
    navLayout->addWidget(m_view);
    navLayout->setSpacing(0);
    navLayout->setContentsMargins(0, 0, 0, 0);
    m_nav->setLayout(navLayout);
}
DBookmarkScene* DLeftSideBar::scene(){
    return m_scene;
}

DToolBar *DLeftSideBar::toolbar() const
{
    return m_toolbar;
}

void DLeftSideBar::setToolbar(DToolBar *toolbar)
{
    m_toolbar = toolbar;
}

void DLeftSideBar::setDisableUrlSchemes(const QList<QString> &schemes)
{
    m_scene->setDisableUrlSchemes(schemes);
}

void DLeftSideBar::resizeEvent(QResizeEvent *e)
{
    QRect rect = geometry();
    if(rect.width() < 70 && !m_isTight)
        toTightNav();
    else if(rect.width() > 70 && m_isTight)
        toNormalNav();
    updateVerticalScrollBar();
    QFrame::resizeEvent(e);
}

void DLeftSideBar::handleLocationChanged(const DFMEvent &e)
{
    if (e.windowId() != WindowManager::getWindowId(this))
        return;

    if (e.fileUrl().isNetWorkFile()) {
        emit fileSignalManager->requestFetchNetworks(DFMUrlBaseEvent(e.sender(), e.fileUrl()));
    } else {
        DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, e.fileUrl(), window());
    }
}

void DLeftSideBar::navSwitched()
{
    if(!m_isTight)
    {
        this->setFixedWidth(LEFTSIDEBAR_MIN_WIDTH);
        m_scene->setTightMode(true);
        m_isTight = true;
    }
    else
    {
        m_isTight = false;
        this->setFixedWidth(LEFTSIDEBAR_MAX_WIDTH);
        m_scene->setTightMode(false);
    }
}

void DLeftSideBar::toTightNav()
{
    m_isTight = true;
    this->setFixedWidth(LEFTSIDEBAR_MIN_WIDTH);
    m_scene->setTightMode(true);
}

void DLeftSideBar::toNormalNav()
{
    m_isTight = false;
    this->setFixedWidth(LEFTSIDEBAR_MAX_WIDTH);
    m_scene->setTightMode(false);
}

void DLeftSideBar::doDragEnter()
{
    m_entered = true;
    update();
}

void DLeftSideBar::doDragLeave()
{
    m_entered = false;
    update();
}

void DLeftSideBar::handdleRequestDiskInfosFinihsed()
{
    const DUrl &currentUrl = WindowManager::getUrlByWindowId(WindowManager::getWindowId(this));

    if (currentUrl.isValid()) {
        if (deviceListener->getDeviceMediaType(currentUrl.path()) == UDiskDeviceInfo::removable){
            if (m_toolbar){
                m_toolbar->setCrumb(currentUrl);
                DBookmarkItem* item =  m_scene->hasBookmarkItem(currentUrl);
                if (item){
                    item->setChecked(true);
                }
            }
        }
    }
}

void DLeftSideBar::handleUserShareCountChanged(const int &count)
{
    Q_UNUSED(count)
    DBookmarkItem* item = m_scene->hasBookmarkItem(DUrl(USERSHARE_ROOT));
    if(item){
        if(userShareManager->hasValidShareFolders())
            item->show();
        else
            item->hide();
    }
}

void DLeftSideBar::centerOnMyShareItem(const QString &path)
{
    Q_UNUSED(path)

    DBookmarkItem* item = m_scene->hasBookmarkItem(DUrl(USERSHARE_ROOT));
    if(item){
        m_view->centerOn(item);
    }
}

QPoint DLeftSideBar::getMyShareItemCenterPos()
{
    DBookmarkItem* item = m_scene->hasBookmarkItem(DUrl(USERSHARE_ROOT));
    if(item){
        m_view->centerOn(item);
        return mapToGlobal(m_view->mapFromScene(item->x() + 80,
                                                item->y()));
    }
    return QPoint(0, 0);
}

void DLeftSideBar::playtShareAddedAnimation()
{
    DBookmarkItem* item = m_scene->hasBookmarkItem(DUrl(USERSHARE_ROOT));
    if(item)
        item->playAnimation();
}

void DLeftSideBar::updateVerticalScrollBar()
{
    m_verticalScrollBar->setFixedSize(8, m_view->height());
    m_verticalScrollBar->move(width() - m_verticalScrollBar->width(), 0);
    if(m_verticalScrollBar->maximum() <= 0){
        m_verticalScrollBar->hide();
    } else{
        m_verticalScrollBar->show();
        m_verticalScrollBar->raise();
    }
}

void DLeftSideBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen;
    if (m_entered){
        pen.setColor(QColor("#2ca7f8"));
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawRect(geometry());
    }else{
        pen.setWidth(0);
    }
}

void DLeftSideBar::loadBookmark()
{
    QList<BookMarkPointer> m_list = bookmarkManager->getBookmarks();
    if(m_list.size())
    {
        m_scene->addSeparator();
    }
    for(int i = 0; i < m_list.size(); i++)
    {
        BookMarkPointer bm = m_list.at(i);
        DBookmarkItem * item = m_scene->createCustomBookmark(bm->getName(), bm->getUrl());
        item->setIsCustomBookmark(true);
        item->setBookmarkModel(bm);
        m_scene->addItem(item);
    }
}

void DLeftSideBar::addNetworkBookmarkItem()
{
    if (DFMGlobal::isRootUser()){
        m_scene->addSeparator();
        return;
    };
    QString key = "Network";
    m_scene->addSeparator();
    DBookmarkItem * item = m_scene->createBookmarkByKey(key);
    item->setUrl(DUrl("network:///"));
    m_scene->addItem(item);
    m_scene->setNetworkDiskItem(item);
}

void DLeftSideBar::addUserShareBookmarkItem()
{
    QString key = "UserShare";
    DBookmarkItem * item = m_scene->createBookmarkByKey(key);
    item->setUrl(DUrl(USERSHARE_ROOT));
    m_scene->addItem(item);
    if(!userShareManager->hasValidShareFolders())
        item->hide();

}

void DLeftSideBar::loadDevices()
{
    foreach (UDiskDeviceInfoPointer device, deviceListener->getDeviceList()) {
        m_scene->mountAdded(device);
    }
}

void DLeftSideBar::loadPluginBookmarks()
{
    foreach (ViewInterface* viewInterface,PluginManager::instance()->getViewInterfaces()) {
        if (viewInterface->isAddSeparator()){
            m_scene->addSeparator();
        }
        QString name = viewInterface->bookMarkText();
        DUrl url = DUrl::fromUserInput(viewInterface->scheme() + ":///");
        qDebug() << viewInterface->scheme() << url;
        DBookmarkItem * item = m_scene->createCustomBookmark(name, url);
        item->setReleaseIcon(viewInterface->bookMarkNormalIcon());
        item->setHoverIcon(viewInterface->bookMarkHoverIcon());
        item->setPressedIcon(viewInterface->bookMarkPressedIcon());
        item->setCheckedIcon(viewInterface->bookMarkCheckedIcon());
        item->setDefaultItem(true);
        m_scene->addItem(item);
    }
}
QGraphicsView *DLeftSideBar::view() const
{
    return m_view;
}
