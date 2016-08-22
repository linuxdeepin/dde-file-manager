#include "dfilemanagerwindow.h"
#include "dleftsidebar.h"
#include "dtoolbar.h"
#include "dfileview.h"
#include "ddetailview.h"
#include "dfilemenu.h"
#include "dsearchbar.h"
#include "dsplitter.h"
#include "extendview.h"
#include "dstatusbar.h"
#include "filemenumanager.h"
#include "computerview.h"

#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filesignalmanager.h"

#include "utils/xutil.h"
#include "utils/xutil.h"
#include "utils/utils.h"

#include "widgets/singleton.h"

#include <QStatusBar>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QResizeEvent>
#include <QThread>
#include <QDesktopWidget>
#include <QStackedLayout>
#include <QTabBar>

const int DFileManagerWindow::MinimumWidth = 0;

DFileManagerWindow::DFileManagerWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));
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
    m_titleBar->layout()->setContentsMargins(0, 0, 8, 0);
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
    initTabBar();
    initViewLayout();
    initFileView();
    initComputerView();

    m_rightView = new QFrame;

    m_titleFrame = new QFrame;
    m_titleFrame->setObjectName("TitleBar");
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

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_titleFrame);
    mainLayout->addWidget(m_tabBar);
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

void DFileManagerWindow::initTabBar()
{
    m_tabBar = new QTabBar(this);
    m_tabBar->setFixedHeight(30);
    m_tabBar->setExpanding(true);
    m_tabBar->hide();
}

void DFileManagerWindow::initViewLayout()
{
    m_viewStackLayout = new QStackedLayout(this);
    m_viewStackLayout->setSpacing(0);
    m_viewStackLayout->setContentsMargins(0, 0, 0, 0);
}

void DFileManagerWindow::initFileView()
{
    FMEvent event;
    event = DUrl();
    event = windowId();
    createNewView(event);
}

void DFileManagerWindow::initComputerView()
{
    m_computerView = new ComputerView(this);
    m_viewStackLayout->addWidget(m_computerView);
    m_views.insert(ComputerView::url(), m_computerView);
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

    connect(fileSignalManager, &FileSignalManager::requestOpenInNewTab, this, &DFileManagerWindow::openNewTab);

    connect(fileSignalManager, &FileSignalManager::fetchNetworksSuccessed, this, &DFileManagerWindow::cd);
    connect(fileSignalManager, &FileSignalManager::requestChangeCurrentUrl,this, &DFileManagerWindow::preHandleCd);
}

DUrl DFileManagerWindow::currentUrl() const
{
    if (m_viewStackLayout->currentWidget() == m_fileView){
        return m_fileView->currentUrl();
    }else if (m_viewStackLayout->currentWidget() == m_computerView){
        return DUrl::fromComputerFile("/");
    }
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

DToolBar *DFileManagerWindow::getToolBar()
{
    return m_toolbar;
}

int DFileManagerWindow::windowId()
{
    return window()->winId();
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
    m_fileView->sortByRole(sortRole);
}

void DFileManagerWindow::setIconView()
{
    m_viewStackLayout->setCurrentWidget(m_fileView);
    m_fileView->setViewModeToIcon();
}

void DFileManagerWindow::setListView()
{
    m_viewStackLayout->setCurrentWidget(m_fileView);
    m_fileView->setViewModeToList();
}

void DFileManagerWindow::preHandleCd(const FMEvent &event)
{
    qDebug() << event << windowId();
    if (event.windowId() != windowId()){
        return;
    }
    if (event.fileUrl().isNetWorkFile()){
        emit fileSignalManager->requestFetchNetworks(event);
    }else if (event.fileUrl().isSMBFile()){
        emit fileSignalManager->requestFetchNetworks(event);
    }else if (event.fileUrl().isComputerFile()){
        showComputerView(event);
    }else if (event.fileUrl().isSearchFile() && m_viewStackLayout->currentWidget() == m_computerView){
        return;
    }else{
        cd(event);
    }
}

void DFileManagerWindow::cd(const FMEvent &event)
{
    if (m_viewStackLayout->currentWidget() != m_fileView){
        m_viewStackLayout->setCurrentWidget(m_fileView);
    }
    m_fileView->cd(event);
    m_toolbar->setViewModeButtonVisible(true);
}

void DFileManagerWindow::showComputerView(const FMEvent &event)
{
    m_viewStackLayout->setCurrentWidget(m_computerView);
    emit fileSignalManager->currentUrlChanged(event);
    m_toolbar->setViewModeButtonVisible(false);
}

void DFileManagerWindow::openNewTab(const FMEvent &event)
{
    if (event.windowId() != windowId()){
        return;
    }
    qDebug() << event;
    createNewView(event);

    if (m_views.count() >= 3){
        m_tabBar->show();
    }
}

void DFileManagerWindow::createNewView(const FMEvent &event)
{
    qDebug() << event;
    DFileView* view = new DFileView(this);
    view->setObjectName("FileView");
    setFocusProxy(view);
    m_views.insert(event.fileUrl(), view);

    m_viewStackLayout->addWidget(view);

    m_viewStackLayout->setCurrentWidget(view);

    view->cd(event);
    m_fileView = view;

    m_tabBar->addTab(event.fileUrl().toString());
    connect(view, &DFileView::viewModeChanged, m_toolbar, &DToolBar::checkViewModeButton);
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
    setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    m_fileManagerWindow = new DFileManagerWindow(this);
    addContenWidget(m_fileManagerWindow);
}

void DMainWindow::initConnect()
{

}


void DMainWindow::moveCenter(const QPoint &cp){
    QRect qr = frameGeometry();

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

void DMainWindow::mousePressEvent(QMouseEvent *event)
{
    m_fileManagerWindow->getToolBar()->getSearchBar()->hideCompleter();
    DWindowFrame::mousePressEvent(event);
}

void DMainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (this->isActiveWindow()){
        if (event->y() <= m_fileManagerWindow->getTitleBar()->height() + layoutMargin + 2) {
            if (event->buttons()) {
                Qt::MouseButton button = event->buttons() & Qt::LeftButton ? Qt::LeftButton :
                    event->buttons() & Qt::RightButton ? Qt::RightButton :
                    Qt::NoButton;
                startMoving(button);
            }
        }
    }
    DWindowFrame::mouseMoveEvent(event);
}

void DMainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->y() <= m_fileManagerWindow->getTitleBar()->height() + layoutMargin + 2 && event->button() == Qt::LeftButton){
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
