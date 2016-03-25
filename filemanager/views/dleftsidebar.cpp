#include "dleftsidebar.h"
#include "../app/global.h"
#include "../app/fmevent.h"
#include "dcheckablebutton.h"
#include "dhorizseparator.h"
#include "dscrollbar.h"

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
    m_iconlist << ":/icons/images/icons/sidebar_expand_normal.png" //file
               << ":/icons/images/icons/folder-recent-symbolic.svg" //recent
               << ":/icons/images/icons/user-home-symbolic.svg" //home
               << ":/icons/images/icons/folder-desktop-symbolic.svg" //desktop
               << ":/icons/images/icons/folder-videos-symbolic.svg" //video
               << ":/icons/images/icons/folder-music-symbolic.svg" //music
               << ":/icons/images/icons/folder-pictures-symbolic.svg" //picture
               << ":/icons/images/icons/folder-documents-symbolic.svg" //document
               << ":/icons/images/icons/folder-download-symbolic.svg" //download

               << ":/icons/images/icons/user-trash-symbolic.svg" //trash
               << ":/icons/images/icons/drive-removable-media-symbolic.svg" //disk
               << ":/images/images/dark/appbar.iphone.png" //my mobile
               << ":/icons/images/icons/user-bookmarks-symbolic.svg";//bookmarks

    m_nameList << "File" << "Recent" << "Home"  << "Desktop"
               << "Videos" << "Musics" << "Pictures" << "Documents" << "Downloads"
               << "Trash" << "Disks" << "My Mobile" << "Bookmarks" ;
    m_navState = true;
}

void DLeftSideBar::initUI()
{
    initTightNav();
    initNav();
    m_stackedWidget = new QStackedWidget;
    m_stackedWidget->addWidget(m_tightNav);
    m_stackedWidget->addWidget(m_nav);


    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    m_stackedWidget->setCurrentIndex(1);
}

void DLeftSideBar::initConnect()
{
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleLocationChanged(int)));
    connect(m_tightNavButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleLocationChanged(int)));
    connect(m_fileButton, &DCheckableButton::released, this, &DLeftSideBar::toTightNav);
    connect(m_tightNavFileButton, &DCheckableButton::released, this, &DLeftSideBar::toNormalNav);
}

void DLeftSideBar::initTightNav()
{
    m_tightNav = new QFrame(this);

    m_tightNavFileButton = new QPushButton("", this);
    m_tightNavFileButton->setObjectName("FileButton");
    m_tightNavFileButton->setFixedSize(QSize(56, 42));
    m_tightNavFileButton->setFocusPolicy(Qt::NoFocus);

    m_tightNavButtonGroup = new QButtonGroup;
    QVBoxLayout * tightNavLayout = new QVBoxLayout;
    tightNavLayout->addWidget(m_tightNavFileButton);
    tightNavLayout->setContentsMargins(0, 5, 0, 0);
    tightNavLayout->setSpacing(0);
    m_tightNav->setLayout(tightNavLayout);

    QListWidget * list = new QListWidget;
    list->setFocusPolicy(Qt::NoFocus);
    list->setObjectName("ListWidget");
    for(int i = 1; i < m_iconlist.size(); i++)
    {
        DCheckableButton * button = new DCheckableButton(m_iconlist.at(i), "");
        QListWidgetItem * item = new QListWidgetItem(list);
        item->setSizeHint(QSize(30, 30));
        list->setItemWidget(item, button);
        m_tightNavButtonGroup->addButton(button, i);
    }
    tightNavLayout->addWidget(list);
}

void DLeftSideBar::initNav()
{
    m_nav = new QFrame;
    QVBoxLayout* navLayout = new QVBoxLayout;
    m_nav->setLayout(navLayout);
    //add file button
    QHBoxLayout * fileButtonLayout = new QHBoxLayout;
    QLabel * fileLabel = new QLabel(tr("File"));
    fileLabel->setObjectName("FileLabel");
    m_fileButton = new QPushButton("");
    m_fileButton->setObjectName("FileButton");
    m_fileButton->setFixedSize(QSize(22,22));
    m_fileButton->setFocusPolicy(Qt::NoFocus);
    fileButtonLayout->addWidget(m_fileButton);
    fileButtonLayout->addWidget(fileLabel);
    fileButtonLayout->setContentsMargins(17, 15, 0, 10);
    fileButtonLayout->setSpacing(8);

    navLayout->addLayout(fileButtonLayout);
    navLayout->setSpacing(0);
    navLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonGroup = new QButtonGroup;
    m_listWidget = new QListWidget;
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    m_listWidget->setObjectName("ListWidget");
    DScrollBar * scrollbar = new DScrollBar;
    m_listWidget->setVerticalScrollBar(scrollbar);
    m_listWidget->setAutoScroll(true);

    //recent, home, desktop, video, pcitures, documents, download, musics, trash
    for(int i = 1; i <= 9; i++)
    {
        DCheckableButton * button = new DCheckableButton(m_iconlist.at(i), m_nameList.at(i));
        QListWidgetItem * item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(110, 30));
        m_listWidget->setItemWidget(item, button);
        m_buttonGroup->addButton(button, i);
    }

    //add separator line
    QListWidgetItem * item = new QListWidgetItem(m_listWidget);
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(QSize(110, 3));
    m_listWidget->setItemWidget(item, new DHorizSeparator);

    //disk, my mobile
    for(int i = 10; i < m_iconlist.size() - 1; i++)
    {
        DCheckableButton * button = new DCheckableButton(m_iconlist.at(i), m_nameList.at(i));
        QListWidgetItem * item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(110, 30));
        m_listWidget->setItemWidget(item, button);
        m_buttonGroup->addButton(button, i);
    }

    //add separator line
    item = new QListWidgetItem(m_listWidget);
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(QSize(110, 3));
    m_listWidget->setItemWidget(item, new DHorizSeparator);

    //add bookmark item
    int i = m_iconlist.size() - 1;
    DCheckableButton * button = new DCheckableButton(m_iconlist.at(i), m_nameList.at(i));
    item = new QListWidgetItem(m_listWidget);
    item->setSizeHint(QSize(110, 30));
    m_listWidget->setItemWidget(item, button);
    m_buttonGroup->addButton(button, i);

//    for(int i = 12; i < 30; i++)
//    {
//        DCheckableButton * button = new DCheckableButton(":/images/images/dark/appbar.app.favorite.png", "label");
//        QListWidgetItem * item = new QListWidgetItem(m_listWidget);
//        item->setSizeHint(QSize(110, 30));
//        m_listWidget->setItemWidget(item, button);
//        m_buttonGroup->addButton(button, i);
//    }
    navLayout->addWidget(m_listWidget);
}

QString DLeftSideBar::getStandardPathbyId(int id)
{
    QString path;
    switch (id) {
    case 1:
        path = Recent;
        break;
    case 2:
        path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
        break;
    case 3:
        path = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
        break;
    case 4:
        path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0);
        break;
    case 5:
        path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0);
        break;
    case 6:
        path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
        break;
    case 7:
        path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
        break;
    case 8:
        path = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).at(0);
        break;
    case 9:
        path = TrashDir;
        break;
    case 10:
        path = QDir::rootPath();
        break;
    default:
        break;
    }
    return path;
}

void DLeftSideBar::handleLocationChanged(int id)
{
    if (id == 1){

    }else{
        FMEvent event;

        event.dir = getStandardPathbyId(id);
        event.source = FMEvent::FileView;

        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}

void DLeftSideBar::toTightNav()
{
    m_stackedWidget->setCurrentIndex(0);
    this->setFixedWidth(60);
}

void DLeftSideBar::toNormalNav()
{
    m_stackedWidget->setCurrentIndex(1);
    this->setFixedWidth(160);
}


