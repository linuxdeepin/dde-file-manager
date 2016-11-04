#include "dleftsidebar.h"
#include "dcheckablebutton.h"
#include "dhorizseparator.h"
#include "windowmanager.h"
#include "dfilemanagerwindow.h"

#include "controllers/bookmarkmanager.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "usershare/usersharemanager.h"

#include "deviceinfo/udisklistener.h"

#include "widgets/singleton.h"

#include "dhoverbutton.h"
#include "dbookmarkitem.h"
#include "dbookmarkscene.h"
#include "dbookmarkitemgroup.h"
#include "dbookmarkrootitem.h"
#include "dfileview.h"
#include "dtoolbar.h"

#include <dscrollbar.h>

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
               << "Disk";

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

    m_view->centerOn(0,0);
}

void DLeftSideBar::initConnect()
{
//    connect(m_fileButton, &DCheckableButton::released, this, &DLeftSideBar::navSwitched);
    connect(m_scene->getGroup(), &DBookmarkItemGroup::url, this, &DLeftSideBar::handleLocationChanged);
    connect(m_scene, &DBookmarkScene::dragEntered, this, &DLeftSideBar::doDragEnter);
    connect(m_scene, &DBookmarkScene::dragLeft, this, &DLeftSideBar::doDragLeave);
    connect(m_scene, &DBookmarkScene::dropped, this, &DLeftSideBar::doDragLeave);
    connect(deviceListener, &UDiskListener::requestDiskInfosFinihsed, this, &DLeftSideBar::handdleRequestDiskInfosFinihsed);
    connect(fileSignalManager, &FileSignalManager::userShareCountChanged, this, &DLeftSideBar::handleUserShareCountChanged);
}

void DLeftSideBar::initNav()
{
    m_nav = new QFrame;
    m_nav->setStyleSheet("border:0px solid red");
    m_nav->setFocusPolicy(Qt::NoFocus);
    m_nav->setFixedWidth(200);
    QVBoxLayout* navLayout = new QVBoxLayout;

    m_view = new QGraphicsView;
    m_view->setAcceptDrops(true);
    m_view->setVerticalScrollBar(new DScrollBar);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setObjectName("Bookmark");
    m_view->setAlignment(Qt::AlignTop);
    m_scene = new DBookmarkScene(this);
    m_scene->setSceneRect(10, 10, 200, 500);
    m_view->setScene(m_scene);

    m_view->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    foreach (QString key, m_nameList) {
        if (key == "Separator"){
            m_scene->addSeparator();
        }else{
            DBookmarkItem * item = m_scene->createBookmarkByKey(key);
            m_scene->addItem(item);
            if (key == "Home"){
                m_scene->setHomeItem(item);
            }
            if (key == "Disk"){
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
    QFrame::resizeEvent(e);
}

void DLeftSideBar::handleLocationChanged(const DFMEvent &e)
{
    if(e.windowId() != WindowManager::getWindowId(this))
        return;
    DFMEvent event;

    event << e.fileUrl();
    event << DFMEvent::LeftSideBar;
    event << WindowManager::getWindowId(this);
    event.setBookmarkIndex(e.bookmarkIndex());

    if (e.fileUrl().isNetWorkFile()){
        emit fileSignalManager->requestFetchNetworks(e);
    }else{
        emit fileSignalManager->requestChangeCurrentUrl(event);
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
        DBookmarkItem * item = DBookmarkItem::makeBookmark(bm->getName(), bm->getUrl());
        item->setBookmarkModel(bm);
        m_scene->addItem(item);
    }
}

void DLeftSideBar::addNetworkBookmarkItem()
{
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
    if (deviceListener->getAllDeviceInfos().count() == 0){
        deviceListener->update();
    }else{
        foreach (UDiskDeviceInfoPointer device, deviceListener->getDeviceList()) {
            m_scene->mountAdded(device);
        }
    }
}
QGraphicsView *DLeftSideBar::view() const
{
    return m_view;
}
