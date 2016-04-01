#include "dfilemanagerwindow.h"
#include "utils/xutil.h"
#include "dtitlebar.h"
#include "dleftsidebar.h"
#include "dtoolbar.h"
#include "dfileview.h"
#include "ddetailview.h"
#include "../app/global.h"
#include "../app/fmevent.h"

#include <QStatusBar>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QResizeEvent>
#include "dsearchbar.h"
#include <QThread>

const int DFileManagerWindow::MinimumWidth = 540;

DFileManagerWindow::DFileManagerWindow(QWidget *parent) : DMovableMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowIcon(QIcon(":/images/images/system-file-manager.png"));
    initData();
    initUI();
    initConnect();
}

DFileManagerWindow::~DFileManagerWindow()
{

}

void DFileManagerWindow::initData()
{

}

void DFileManagerWindow::initUI()
{
    resize(950, 600);
    setMinimumWidth(MinimumWidth);
    moveCenter();
    initCentralWidget();
    //initStatusBar();
    setCentralWidget(m_centralWidget);
    setStyleSheet(getQssFromFile(":/qss/qss/filemanager.qss"));
}

void DFileManagerWindow::initTitleBar()
{
    m_titleBar = new DTitleBar;
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->setFixedHeight(40);
    m_titleBar->setFocusPolicy(Qt::ClickFocus);
    setDragMovableHeight(m_titleBar->height());
}

void DFileManagerWindow::initSplitter()
{
    initLeftSideBar();
    initRightView();

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setFixedWidth(5);
    m_splitter->addWidget(m_leftSideBar);
    m_splitter->addWidget(m_rightView);
}

void DFileManagerWindow::initLeftSideBar()
{
    m_leftSideBar = new DLeftSideBar(this);
    m_leftSideBar->setObjectName("LeftSideBar");
    m_leftSideBar->setMinimumWidth(30);
    m_leftSideBar->setMaximumWidth(200);
    m_leftSideBar->setFocusPolicy(Qt::ClickFocus);
}

void DFileManagerWindow::initRightView()
{
    initToolBar();
    initFileView();
    initDetailView();
    m_rightView = new QFrame;

    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->addWidget(m_toolbar);
    titleLayout->addWidget(m_titleBar);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* viewLayout = new QHBoxLayout;
    viewLayout->addWidget(m_fileView);
    viewLayout->addWidget(m_detailView);
    viewLayout->setSpacing(0);
    viewLayout->setContentsMargins(0, 0, 0, 0);

    m_statusBar = new QStatusBar;
    m_statusBar->setFixedHeight(20);
    m_statusBar->setFocusPolicy(Qt::ClickFocus);
    m_statusBar->setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(titleLayout);
    mainLayout->addLayout(viewLayout);
    mainLayout->addWidget(m_statusBar);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_rightView->setLayout(mainLayout);
}

void DFileManagerWindow::initToolBar()
{
    m_toolbar = new DToolBar(this);
    m_toolbar->setObjectName("ToolBar");
    m_toolbar->setFixedHeight(40);
    m_toolbar->setFocusPolicy(Qt::ClickFocus);
}

void DFileManagerWindow::initFileView()
{
    m_fileView = new DFileView(this);
    m_fileView->setObjectName("FileView");
}

void DFileManagerWindow::initDetailView()
{
    m_detailView = new DDetailView(this);
    m_detailView->setObjectName("DetailView");
    m_detailView->setFixedWidth(200);
    m_detailView->hide();
}

void DFileManagerWindow::initCentralWidget()
{
    initTitleBar();
    initSplitter();

    m_centralWidget = new QFrame(this);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_splitter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_centralWidget->setLayout(mainLayout);
}

void DFileManagerWindow::initStatusBar()
{
    m_statusBar = new QStatusBar(this);
    m_statusBar->setFixedHeight(30);
    m_statusBar->setFocusPolicy(Qt::ClickFocus);
    setStatusBar(m_statusBar);
}

void DFileManagerWindow::initConnect()
{
    connect(m_titleBar, &DTitleBar::minimuned, this, &DFileManagerWindow::showMinimized);
    connect(m_titleBar, &DTitleBar::switchMaxNormal, this, &DFileManagerWindow::toggleMaxNormal);
    connect(m_titleBar, &DTitleBar::closed, this, &DFileManagerWindow::close);
    connect(m_toolbar, &DToolBar::requestListView, m_fileView, &DFileView::switchToListMode);
    connect(m_toolbar, &DToolBar::requestIconView, m_fileView, &DFileView::switchToIconMode);
}

void DFileManagerWindow::toggleMaxNormal()
{
    if (isMaximized()){
        showNormal();
        m_titleBar->setNormalIcon();
    }else{
        showMaximized();
        m_titleBar->setMaxIcon();
    }
}


void DFileManagerWindow::resizeEvent(QResizeEvent *event)
{
    DMovableMainWindow::resizeEvent(event);
}

void DFileManagerWindow::keyPressEvent(QKeyEvent *e)
{
    m_toolbar->m_searchBar->setFocus();
    m_toolbar->m_searchBar->keyPressEvent(e);
    DMovableMainWindow::keyPressEvent(e);
}

