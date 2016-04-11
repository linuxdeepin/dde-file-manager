#include "dfilemanagerwindow.h"
#include "utils/xutil.h"
#include "dleftsidebar.h"
#include "dtoolbar.h"
#include "dfileview.h"
#include "ddetailview.h"
#include "../app/global.h"
#include "../app/fmevent.h"
#include "dsearchbar.h"
#include "dsplitter.h"
#include "utils/xutil.h"

#include <QStatusBar>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QResizeEvent>
#include <QThread>



const int DFileManagerWindow::MinimumWidth = 0;

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
    resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    setMinimumWidth(MinimumWidth);
    moveCenter();
    initCentralWidget();
//    initStatusBar();
    setCentralWidget(m_centralWidget);
    setStyleSheet(getQssFromFile(":/qss/qss/filemanager.qss"));
}

void DFileManagerWindow::initTitleBar()
{
    m_titleBar = new DTitlebar;
    m_titleBar->layout()->setContentsMargins(0, 0, 0, 0);
    m_titleBar->setWindowFlags(m_titleBar->windowFlags());
    setDragMovableHeight(m_titleBar->height());
}

void DFileManagerWindow::initSplitter()
{
    initLeftSideBar();
    initRightView();

    m_splitter = new DSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(m_leftSideBar);
    m_splitter->addWidget(m_rightView);
    m_splitter->setChildrenCollapsible(false);

    connect(m_leftSideBar, &DLeftSideBar::moveSplitter, m_splitter, &DSplitter::moveSplitter);
}

void DFileManagerWindow::initLeftSideBar()
{
    m_leftSideBar = new DLeftSideBar(this);
    m_leftSideBar->setObjectName("LeftSideBar");
    m_leftSideBar->setMaximumWidth(LEFTSIDEBAR_MAX_WIDTH);
    m_leftSideBar->setMinimumWidth(LEFTSIDEBAR_MIN_WIDTH);
    m_leftSideBar->setFocusPolicy(Qt::ClickFocus);
}

void DFileManagerWindow::initRightView()
{
    initToolBar();
    initFileView();
    initDetailView();
    m_rightView = new QFrame;

    m_titleFrame = new QFrame;
    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(m_toolbar);
    titleLayout->addWidget(m_titleBar);
    // TODO: why, fixed me by Iceyer
    QWidget *empty = new QWidget;
    empty->setFixedSize(0,1);
    m_titleBar->setCustomWidget(empty, Qt::AlignLeft, false);

    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleFrame->setLayout(titleLayout);

    m_titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);

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
    mainLayout->addWidget(m_titleFrame);
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
    connect(m_titleBar, &DTitlebar::minimumClicked, this, &DFileManagerWindow::showMinimized);
    connect(m_titleBar, &DTitlebar::maximumClicked, this, &DFileManagerWindow::showMaximized);
    connect(m_titleBar, &DTitlebar::restoreClicked, this, &DFileManagerWindow::showNormal);
    connect(m_titleBar, &DTitlebar::closeClicked, this, &DFileManagerWindow::close);
    connect(m_toolbar, &DToolBar::requestListView, m_fileView, &DFileView::switchToListMode);
    connect(m_toolbar, &DToolBar::requestIconView, m_fileView, &DFileView::switchToIconMode);
}

void DFileManagerWindow::showMinimized()
{
    QtX11::utils::ShowMinimizedWindow(this);
}


void DFileManagerWindow::resizeEvent(QResizeEvent *event)
{  
    m_titleFrame->setMaximumWidth(rect().width() - LEFTSIDEBAR_MAX_WIDTH);
    DMovableMainWindow::resizeEvent(event);
}

void DFileManagerWindow::keyPressEvent(QKeyEvent *event)
{
//    m_toolbar->m_searchBar->setFocus();
//    m_toolbar->m_searchBar->keyPressEvent(e);
    DMovableMainWindow::keyPressEvent(event);
}

void DFileManagerWindow::closeEvent(QCloseEvent *event)
{
    emit aboutToClose();
    DMovableMainWindow::closeEvent(event);
}

