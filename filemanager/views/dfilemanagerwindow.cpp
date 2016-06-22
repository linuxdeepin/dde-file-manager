#include "dfilemanagerwindow.h"
#include "utils/xutil.h"
#include "dleftsidebar.h"
#include "dtoolbar.h"
#include "dfileview.h"
#include "ddetailview.h"
#include "../app/global.h"
#include "../app/fmevent.h"
#include "dfilemenu.h"
#include "dsearchbar.h"
#include "dsplitter.h"
#include "utils/xutil.h"
#include "extendview.h"
#include "dstatusbar.h"

#include <QStatusBar>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QResizeEvent>
#include <QThread>
#include <QDesktopWidget>
#include <QStackedLayout>



const int DFileManagerWindow::MinimumWidth = 0;

DFileManagerWindow::DFileManagerWindow(QWidget *parent) : QMainWindow(parent)
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
    setMinimumSize(800, 420);
    initCentralWidget();
    setCentralWidget(m_centralWidget);
    setStyleSheet(getQssFromFile(":/qss/qss/filemanager.qss"));
}

void DFileManagerWindow::initTitleBar()
{
    m_titleBar = new DTitlebar(this);
    m_titleBar->layout()->setContentsMargins(0, 0, 5, 0);
    m_titleBar->setWindowFlags(m_titleBar->windowFlags());

    DFileMenu* menu = fileMenuManger->createToolBarSettingsMenu();

    FMEvent event;
    event = window()->winId();
    menu->setEvent(event);

    m_titleBar->setMenu(menu);
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
    m_leftSideBar->setFixedWidth(LEFTSIDEBAR_MAX_WIDTH);
}

void DFileManagerWindow::initRightView()
{
    initToolBar();
    initFileView();
    initExtendView();
    m_rightView = new QFrame;

    m_titleFrame = new QFrame;
    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(m_toolbar);
    titleLayout->addWidget(m_titleBar);

    QWidget *empty = new QWidget;
    empty->setFixedSize(0,1);
    m_titleBar->setCustomWidget(empty, Qt::AlignLeft, false);

    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleFrame->setLayout(titleLayout);

    m_titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);

    m_viewStackLayout = new QStackedLayout(this);
    m_viewStackLayout->addWidget(m_fileView);
    m_viewStackLayout->addWidget(m_extendView);
    m_viewStackLayout->setSpacing(0);
    m_viewStackLayout->setContentsMargins(0, 0, 0, 0);

    m_statusBar = new DStatusBar(m_fileView);
    m_fileView->addFooterWidget(m_statusBar);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_titleFrame);
    mainLayout->addLayout(m_viewStackLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_rightView->setLayout(mainLayout);
}

void DFileManagerWindow::initToolBar()
{
    m_toolbar = new DToolBar(this);
    m_toolbar->setObjectName("ToolBar");
    m_toolbar->setFixedHeight(40);
}

void DFileManagerWindow::initFileView()
{
    m_fileView = new DFileView(this);
    m_fileView->setObjectName("FileView");

    setFocusProxy(m_fileView);
}

void DFileManagerWindow::initExtendView()
{
    m_extendView = new ExtendView(this);
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

void DFileManagerWindow::initConnect()
{
    connect(m_titleBar, SIGNAL(minimumClicked()), parentWidget(), SLOT(showMinimized()));
    connect(m_titleBar, SIGNAL(maximumClicked()), parentWidget(), SLOT(showMaximized()));
    connect(m_titleBar, SIGNAL(restoreClicked()), parentWidget(), SLOT(showNormal()));
    connect(m_titleBar, SIGNAL(closeClicked()), parentWidget(), SLOT(close()));
    connect(m_toolbar, &DToolBar::requestIconView, this, &DFileManagerWindow::setIconView);
    connect(m_toolbar, &DToolBar::requestListView, this, &DFileManagerWindow::setListView);
    connect(m_toolbar, &DToolBar::requestExtendView, this, &DFileManagerWindow::setExtendView);
    connect(m_fileView, &DFileView::viewModeChanged, m_toolbar, &DToolBar::checkViewModeButton);
}

DUrl DFileManagerWindow::currentUrl() const
{
    return m_fileView->currentUrl();
}

int DFileManagerWindow::getFileViewMode() const
{
    return m_fileView->getDefaultViewMode();
}

int DFileManagerWindow::getFileViewSortRole() const
{
    return m_fileView->getSortRoles();
}

DTitlebar *DFileManagerWindow::getTitleBar()
{
    return m_titleBar;
}

void DFileManagerWindow::showMinimized()
{
    QtX11::utils::ShowMinimizedWindow(this);
}

void DFileManagerWindow::setFileViewMode(int viewMode)
{
    m_fileView->setViewMode(static_cast<DFileView::ViewMode>(viewMode));
}

void DFileManagerWindow::setFileViewSortRole(int sortRole)
{
    m_fileView->sort(sortRole);
}

void DFileManagerWindow::setIconView()
{
    m_viewStackLayout->setCurrentIndex(0);
    m_fileView->setViewModeToIcon();
}

void DFileManagerWindow::setListView()
{
    m_viewStackLayout->setCurrentIndex(0);
    m_fileView->setViewModeToList();
}

void DFileManagerWindow::setExtendView()
{
    m_viewStackLayout->setCurrentIndex(1);
    m_extendView->setStartUrl(m_fileView->currentUrl());
    m_toolbar->checkViewModeButton(DFileView::ExtendMode);
}


DMainWindow::DMainWindow(QWidget *parent):
    DWindowFrame(parent)
{
    initUI();
    initConnect();
//    startTimer(4000);
}

DMainWindow::~DMainWindow()
{

}

void DMainWindow::initUI()
{
    setFocusPolicy(Qt::NoFocus);
    resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    m_fileManagerWindow = new DFileManagerWindow(this);
    addContenWidget(m_fileManagerWindow);

}

void DMainWindow::initConnect()
{

}


void DMainWindow::moveCenter(){
    QRect qr = frameGeometry();
    QPoint cp;
    cp = qApp->desktop()->availableGeometry().center();
    qr.moveCenter(cp);
    move(qr.topLeft());
}

void DMainWindow::moveTopRight(){
    QRect pRect;
    pRect = qApp->desktop()->availableGeometry();
    int x = pRect.width() - width();
    move(QPoint(x, 0));
}

void DMainWindow::moveTopRightByRect(QRect rect){
    int x = rect.x() + rect.width() - width();
    move(QPoint(x, 0));
}

void DMainWindow::closeEvent(QCloseEvent *event)
{
    emit aboutToClose();
    DWindowFrame::closeEvent(event);
}

void DMainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    qDebug() << qApp->focusWidget();
}

void DMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->y() <= m_fileManagerWindow->getTitleBar()->height() + layoutMargin + 2)
        emit startMoving();
    DWindowFrame::mouseMoveEvent(event);
}

void DMainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->y() <= m_fileManagerWindow->getTitleBar()->height() + layoutMargin + 2){
        if (windowState() != Qt::WindowMaximized){
            setWindowState(Qt::WindowMaximized);
            m_fileManagerWindow->getTitleBar()->setWindowState(Qt::WindowMaximized);
        }else{
            showNormal();
            m_fileManagerWindow->getTitleBar()->setWindowState(Qt::WindowMinimized);
        }
    }
    DWindowFrame::mouseDoubleClickEvent(event);
}

DFileManagerWindow *DMainWindow::fileManagerWindow() const
{
    return m_fileManagerWindow;
}

void DMainWindow::moveCenterByRect(QRect rect){
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}
