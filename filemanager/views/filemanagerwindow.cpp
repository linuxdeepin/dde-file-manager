#include "filemanagerwindow.h"
#include <QStatusBar>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QResizeEvent>

#include "utils/xutil.h"

#include "dtitlebar.h"
#include "dleftsidebar.h"
#include "dtoolbar.h"
#include "dfileview.h"
#include "ddetailview.h"
#include "../app/global.h"

const int FileManagerWindow::MinimumWidth = 540;

FileManagerWindow::FileManagerWindow(QWidget *parent) : DMovableMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowIcon(QIcon(":/images/images/system-file-manager.png"));
    initData();
    initUI();
    initConnect();
}

FileManagerWindow::~FileManagerWindow()
{

}

void FileManagerWindow::initData()
{

}

void FileManagerWindow::initUI()
{
    resize(950, 700);
    setMinimumWidth(MinimumWidth);
    moveCenter();
    initCentralWidget();
    initStatusBar();
    setCentralWidget(m_centralWidget);
    setStyleSheet(getQssFromFile(":/qss/qss/filemanager.qss"));
}

void FileManagerWindow::initTitleBar()
{
    m_titleBar = new DTitleBar(this);
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->setFixedHeight(42);
    m_titleBar->setFocusPolicy(Qt::ClickFocus);
    setDragMovableHeight(m_titleBar->height());
}

void FileManagerWindow::initSplitter()
{
    initLeftSideBar();
    initRightView();

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setFixedWidth(5);
    m_splitter->addWidget(m_leftSideBar);
    m_splitter->addWidget(m_rightView);
}

void FileManagerWindow::initLeftSideBar()
{
    m_leftSideBar = new DLeftSideBar(this);
    m_leftSideBar->setObjectName("LeftSideBar");
    m_leftSideBar->setMinimumWidth(100);
    m_leftSideBar->setMaximumWidth(200);
    m_leftSideBar->setFixedWidth(160);
    m_leftSideBar->setFocusPolicy(Qt::ClickFocus);
}

void FileManagerWindow::initRightView()
{
    initToolBar();
    initFileView();
    initDetailView();
    m_rightView = new QFrame;

    QHBoxLayout* viewLayout = new QHBoxLayout;
    viewLayout->addWidget(m_fileView);
    viewLayout->addWidget(m_detailView);
    viewLayout->setSpacing(0);
    viewLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_toolbar);
    mainLayout->addLayout(viewLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_rightView->setLayout(mainLayout);
}

void FileManagerWindow::initToolBar()
{
    m_toolbar = new DToolBar(this);
    m_toolbar->setObjectName("ToolBar");
    m_toolbar->setFixedHeight(82);
    m_toolbar->setFocusPolicy(Qt::ClickFocus);
}

void FileManagerWindow::initFileView()
{
    m_fileView = new DFileView(this);
    m_fileView->setObjectName("FileView");
}

void FileManagerWindow::initDetailView()
{
    m_detailView = new DDetailView(this);
    m_detailView->setObjectName("DetailView");
    m_detailView->setFixedWidth(200);
    m_detailView->hide();
}

void FileManagerWindow::initCentralWidget()
{
    initTitleBar();
    initSplitter();

    m_centralWidget = new QFrame(this);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_splitter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_centralWidget->setLayout(mainLayout);
}

void FileManagerWindow::initStatusBar()
{
    m_statusBar = new QStatusBar(this);
    m_statusBar->setFixedHeight(30);
    m_statusBar->setFocusPolicy(Qt::ClickFocus);
    setStatusBar(m_statusBar);
}

void FileManagerWindow::initConnect()
{
    connect(m_titleBar, SIGNAL(minimuned()), this, SLOT(showMinimized()));
    connect(m_titleBar, SIGNAL(switchMaxNormal()), this, SLOT(toggleMaxNormal()));
    connect(m_titleBar, SIGNAL(closed()), this, SLOT(close()));
    connect(m_toolbar, SIGNAL(requestSwitchLayout()), this, SLOT(toggleLayout()));
    connect(m_toolbar, &DToolBar::backButtonClicked,
            this, [this] {
        QDir dir(m_fileView->currentUrl());

        dir.cdUp();

        emit fileSignalManager->currentUrlChanged(dir.absolutePath());
    });
    connect(m_toolbar, &DToolBar::switchLayoutMode,
            m_fileView, &DFileView::switchListMode);
    connect(m_toolbar, &DToolBar::refreshButtonClicked,
            this, [this] {
        emit fileSignalManager->refreshFolder(m_fileView->currentUrl());
    });
}

void FileManagerWindow::toggleMaxNormal()
{
    if (isMaximized()){
        showNormal();
        m_titleBar->setNormalIcon();
    }else{
        showMaximized();
        m_titleBar->setMaxIcon();
    }
}

void FileManagerWindow::toggleLayout()
{
    m_leftSideBar->setVisible(not m_leftSideBar->isVisible());
}

void FileManagerWindow::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() <= FileManagerWindow::MinimumWidth){
//        if (m_toolbar->getLayoutbuttonState() == DStateButton::stateA){
//            if (m_leftSideBar->isVisible() && event->size().width() < event->oldSize().width()){
//                m_leftSideBar->hide();
//                m_toolbar->setLayoutButtonState(DStateButton::stateB);
//            }
//        }
    }else{
//        if (m_toolbar->getLayoutbuttonState() == DStateButton::stateB){
//            if (!m_leftSideBar->isVisible()){
//                m_leftSideBar->show();
//            }

//        }
    }
    DMovableMainWindow::resizeEvent(event);
}
