#include "dleftsidebar.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include "../app/global.h"
#include "dcheckablebutton.h"

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

}

void DLeftSideBar::initUI()
{
    m_buttonGroup = new QButtonGroup(this);
    initHomeBar();
    initCommonFolderBar();
    initDiskBar();
    initNetWorkBar();
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_homeBar);
    mainLayout->addWidget(m_commonFolderBar);
    mainLayout->addWidget(m_diskBar);
    mainLayout->addWidget(m_networkBar);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

void DLeftSideBar::initHomeBar()
{
    m_homeBar = new QFrame(this);
    m_homeBar->setObjectName("LeftSideHomeBar");
    m_homeBar->setMaximumHeight(100);
    m_homeButton = new DCheckableButton(":/images/images/dark/appbar.home.png", tr("Home"), this);
    m_historyButton = new DCheckableButton(":/images/images/dark/appbar.clock.png", tr("History"), this);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_homeButton);
    mainLayout->addWidget(m_historyButton);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_homeBar->setLayout(mainLayout);

    m_buttonGroup->addButton(m_homeButton, 0);
    m_buttonGroup->addButton(m_historyButton, 1);
}

void DLeftSideBar::initCommonFolderBar()
{
    m_commonFolderBar = new QFrame(this);
    m_commonFolderBar->setObjectName("LeftSideCommonFolderBar");

    m_desktopButton = new DCheckableButton(":/images/images/dark/appbar.app.png", tr("Desktop"), this);
    m_videoButton = new DCheckableButton(":/images/images/dark/appbar.film.png", tr("Video"), this);
    m_musicButton = new DCheckableButton(":/images/images/dark/appbar.music.png", tr("Music"), this);
    m_pictureButton = new DCheckableButton(":/images/images/dark/appbar.image.png", tr("Picture"), this);
    m_docmentButton = new DCheckableButton(":/images/images/dark/appbar.page.png", tr("Docment"), this);
    m_downloadButton = new DCheckableButton(":/images/images/dark/appbar.download.png", tr("Download"), this);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_desktopButton);
    mainLayout->addWidget(m_videoButton);
    mainLayout->addWidget(m_musicButton);
    mainLayout->addWidget(m_pictureButton);
    mainLayout->addWidget(m_docmentButton);
    mainLayout->addWidget(m_downloadButton);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_commonFolderBar->setLayout(mainLayout);

    m_buttonGroup->addButton(m_desktopButton, 2);
    m_buttonGroup->addButton(m_videoButton, 3);
    m_buttonGroup->addButton(m_musicButton, 4);
    m_buttonGroup->addButton(m_pictureButton, 5);
    m_buttonGroup->addButton(m_docmentButton, 6);
    m_buttonGroup->addButton(m_downloadButton, 7);

}

void DLeftSideBar::initDiskBar()
{
    m_diskBar = new QFrame(this);
    m_diskBar->setObjectName("LeftSideDiskBar");
    m_diskBar->setMaximumHeight(100);
    m_computerButton = new DCheckableButton(":/images/images/dark/appbar.laptop.png", tr("Computer"), this);
    m_trashButton = new DCheckableButton(":/images/images/dark/appbar.delete.png", tr("Trash"), this);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_computerButton);
    mainLayout->addWidget(m_trashButton);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_diskBar->setLayout(mainLayout);

    m_buttonGroup->addButton(m_computerButton, 8);
    m_buttonGroup->addButton(m_trashButton, 9);

}

void DLeftSideBar::initNetWorkBar()
{
    m_networkBar = new QFrame(this);
}

void DLeftSideBar::initConnect()
{
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleLocationChanged(int)));
}

QString DLeftSideBar::getStandardPathbyId(int id)
{
    QString path;
    switch (id) {
    case 0:
        path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
        break;
    case 1:
        path = Recent;
        break;
    case 2:
        path = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
        break;
    case 3:
        path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0);
        break;
    case 4:
        path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0);
        break;
    case 5:
        path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
        break;
    case 6:
        path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
        break;
    case 7:
        path = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).at(0);
        break;
    case 8:
        path = QDir::rootPath();
        break;
    case 9:
        path = TrashDir;
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
        QString path = getStandardPathbyId(id);
        emit fileSignalManager->currentUrlChanged(QUrl(path));
        qDebug() << QUrl(path);
    }
}

