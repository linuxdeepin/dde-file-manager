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
#include "dtabbar.h"
#include "dbookmarkscene.h"
#include "windowmanager.h"

#include "app/global.h"
#include "app/fmevent.h"
#include "app/filesignalmanager.h"
#include "app/filemanagerapp.h"

#include "xutil.h"
#include "utils.h"

#include "widgets/singleton.h"
#include "controllers/fileservices.h"

#include <dplatformwindowhandle.h>
#include <DTitlebar>

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

DWIDGET_USE_NAMESPACE;

class DFileManagerWindowPrivate
{
public:
    DFileManagerWindowPrivate(DFileManagerWindow *qq)
        : q_ptr(qq) {}

    QFrame* centralWidget = NULL;
    DLeftSideBar* leftSideBar = NULL;
    QFrame* rightView = NULL;
    DToolBar* toolbar = NULL;
    TabBar* tabBar = NULL;
    QPushButton *newTabButton;
    DFileView* fileView = NULL;
    ComputerView* computerView = NULL;
    DDetailView* detailView = NULL;
    DStatusBar* statusBar = NULL;
    QVBoxLayout* mainLayout = NULL;
    DSplitter* splitter = NULL;
    QFrame * titleFrame = NULL;
    QStackedLayout* viewStackLayout = NULL;

    QMap<DUrl, QWidget*> views;

    DFileManagerWindow *q_ptr;

    D_DECLARE_PUBLIC(DFileManagerWindow)
};

DFileManagerWindow::DFileManagerWindow(QWidget *parent)
    : DMainWindow(parent)
    , d_ptr(new DFileManagerWindowPrivate(this))
{
    setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));

    /// ensure AppController object is defined
    Q_UNUSED(fileManagerApp->getAppController())

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
    D_DC(DFileManagerWindow);

    resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    setMinimumSize(800, 420);
    initCentralWidget();
    setCentralWidget(d->centralWidget);
    setStyleSheet(getQssFromFile(":/qss/qss/filemanager.qss"));
}

void DFileManagerWindow::initTitleBar()
{
    if (!titleBar())
        return;

    DFileMenu* menu = fileMenuManger->createToolBarSettingsMenu();

    FMEvent event;
    event = windowId();
    menu->setEvent(event);

    titleBar()->setMenu(menu);
    titleBar()->setContentsMargins(0, 1, -1, 0);

    QWidget *widget = new QWidget();

    widget->setFixedSize(35, 0);
    titleBar()->setCustomWidget(widget, false);
}

void DFileManagerWindow::initSplitter()
{
    D_D(DFileManagerWindow);

    initLeftSideBar();
    initRightView();

    d->splitter = new DSplitter(Qt::Horizontal, this);
    d->splitter->addWidget(d->leftSideBar);
    d->splitter->addWidget(d->rightView);
    d->splitter->setChildrenCollapsible(false);

    connect(d->leftSideBar, &DLeftSideBar::moveSplitter, d->splitter, &DSplitter::moveSplitter);
}

void DFileManagerWindow::initLeftSideBar()
{
    D_D(DFileManagerWindow);

    d->leftSideBar = new DLeftSideBar(this);
    d->leftSideBar->setObjectName("LeftSideBar");
    d->leftSideBar->setFixedWidth(LEFTSIDEBAR_MAX_WIDTH);
}

void DFileManagerWindow::initRightView()
{
    D_D(DFileManagerWindow);

    initToolBar();
    initTabBar();
    initViewLayout();
    initComputerView();
    initFileView();

    d->rightView = new QFrame;

    d->titleFrame = new QFrame;
    d->titleFrame->setObjectName("TitleBar");
    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(d->toolbar);
    titleLayout->addWidget(titleBar());
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    d->titleFrame->setLayout(titleLayout);
    d->titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);

    QHBoxLayout *tabBarLayout = new QHBoxLayout;
    tabBarLayout->setMargin(0);
    tabBarLayout->setSpacing(0);
    tabBarLayout->addWidget(d->tabBar);
    tabBarLayout->addWidget(d->newTabButton);


    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(d->titleFrame);
    mainLayout->addLayout(tabBarLayout);
    mainLayout->addLayout(d->viewStackLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    d->rightView->setLayout(mainLayout);
}

void DFileManagerWindow::initToolBar()
{
    D_D(DFileManagerWindow);

    d->toolbar = new DToolBar(this);
    d->toolbar->setObjectName("ToolBar");
    d->toolbar->setFixedHeight(40);
}

void DFileManagerWindow::initTabBar()
{
    D_D(DFileManagerWindow);

    d->tabBar = new TabBar(this);
    d->tabBar->setFixedHeight(24);

    d->newTabButton = new QPushButton(this);
    d->newTabButton->setObjectName("NewTabButton");
    d->newTabButton->setFixedSize(25,24);
    d->newTabButton->hide();
}

void DFileManagerWindow::initViewLayout()
{
    D_D(DFileManagerWindow);

    d->viewStackLayout = new QStackedLayout(this);
    d->viewStackLayout->setSpacing(0);
    d->viewStackLayout->setContentsMargins(0, 0, 0, 0);
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
    D_D(DFileManagerWindow);

    d->computerView = new ComputerView(this);
    d->viewStackLayout->addWidget(d->computerView);
    d->views.insert(ComputerView::url(), d->computerView);
}

void DFileManagerWindow::initCentralWidget()
{
    D_D(DFileManagerWindow);

    initTitleBar();
    initSplitter();

    d->centralWidget = new QFrame(this);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(d->splitter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    d->centralWidget->setLayout(mainLayout);
}

void DFileManagerWindow::initConnect()
{
    D_D(DFileManagerWindow);

    if (titleBar()) {
        connect(titleBar(), SIGNAL(minimumClicked()), parentWidget(), SLOT(showMinimized()));
        connect(titleBar(), SIGNAL(maximumClicked()), parentWidget(), SLOT(showMaximized()));
        connect(titleBar(), SIGNAL(restoreClicked()), parentWidget(), SLOT(showNormal()));
        connect(titleBar(), SIGNAL(closeClicked()), parentWidget(), SLOT(close()));
    }

    connect(d->toolbar, &DToolBar::requestIconView, this, &DFileManagerWindow::setIconView);
    connect(d->toolbar, &DToolBar::requestListView, this, &DFileManagerWindow::setListView);

    connect(fileSignalManager, &FileSignalManager::requestOpenInNewTab, this, &DFileManagerWindow::openNewTab);

    connect(fileSignalManager, &FileSignalManager::fetchNetworksSuccessed, this, &DFileManagerWindow::cd);
    connect(fileSignalManager, &FileSignalManager::requestChangeCurrentUrl,this, &DFileManagerWindow::preHandleCd);

    connect(d->tabBar, &TabBar::tabMoved, d->toolbar, &DToolBar::moveNavStacks);
    connect(d->tabBar, &TabBar::currentChanged,this, &DFileManagerWindow::onCurrentTabChanged);
    connect(d->tabBar, &TabBar::tabCloseRequested, this,&DFileManagerWindow::onCurrentTabClosed);
    connect(d->tabBar, &TabBar::tabAddableChanged, this, &DFileManagerWindow::onTabAddableChanged);
    connect(d->newTabButton, &QPushButton::clicked, this, [=]{
        FMEvent event;
        const DUrl url = DUrl::fromUserInput(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0));
        event = url;
        event = windowId();
        createNewView(event);
    });
}

void DFileManagerWindow::onCurrentTabClosed(const int index){

    D_D(DFileManagerWindow);

    int viewIndex = d->tabBar->tabData(index).toJsonObject()["viewIndex"].toInt();
    qDebug()<<"************************delete file view:"<<viewIndex;
    DFileView *currentView =qobject_cast<DFileView*>(d->viewStackLayout->widget(viewIndex));

    currentView->close();
    d->viewStackLayout->removeWidget(currentView);
    d->fileView = NULL;
    delete currentView;

    //recorrect view index
    for(int i = 0; i<d->tabBar->count(); i++){
        int newViewIndex = d->tabBar->tabData(i).toJsonObject()["viewIndex"].toInt();
        if(newViewIndex > viewIndex){
            newViewIndex--;
            QJsonObject tabData = d->tabBar->tabData(i).toJsonObject();
            tabData["viewIndex"] = newViewIndex;
            d->tabBar->setTabData(i,tabData);
        }
    }

    d->toolbar->removeNavStackAt(index);
    d->tabBar->removeTab(index);

    if(d->tabBar->count()<8)
        emit d->tabBar->tabAddableChanged(true);

    if(d->tabBar->count()<2)
        d->tabBar->hide();

    if(d->tabBar->isHidden())
        d->newTabButton->hide();
}

void DFileManagerWindow::onTabAddableChanged(bool addable)
{
    D_D(DFileManagerWindow);

    d->newTabButton->setEnabled(addable);
}
void DFileManagerWindow::onCurrentTabChanged(int tabIndex)
{
    D_D(DFileManagerWindow);

    int viewIndex = d->tabBar->tabData(tabIndex).toJsonObject()["viewIndex"].toInt();
    switchToView(viewIndex);

    d->toolbar->switchHistoryStack(tabIndex,d->fileView->currentUrl());
}


DUrl DFileManagerWindow::currentUrl() const
{
    D_DC(DFileManagerWindow);

    if (d->viewStackLayout->currentWidget() == d->fileView) {
        return d->fileView->currentUrl();
    } else if (d->viewStackLayout->currentWidget() == d->computerView) {
        return DUrl::fromComputerFile("/");
    }

    return DUrl();
}

int DFileManagerWindow::getFileViewMode() const
{
    D_DC(DFileManagerWindow);

    return d->fileView->getDefaultViewMode();
}

int DFileManagerWindow::getFileViewSortRole() const
{
    D_DC(DFileManagerWindow);

    return d->fileView->getSortRoles();
}

DToolBar *DFileManagerWindow::getToolBar() const
{
    D_DC(DFileManagerWindow);

    return d->toolbar;
}

DFileView *DFileManagerWindow::getFileView() const
{
    D_DC(DFileManagerWindow);

    return d->fileView;
}

int DFileManagerWindow::windowId()
{
    return WindowManager::getWindowId(this);
}

void DFileManagerWindow::setFileViewMode(int viewMode)
{
    D_D(DFileManagerWindow);

    d->fileView->setViewMode(static_cast<DFileView::ViewMode>(viewMode));
}

void DFileManagerWindow::setFileViewSortRole(int sortRole)
{
    D_D(DFileManagerWindow);

    d->fileView->sortByRole(sortRole);
}

void DFileManagerWindow::setIconView()
{
    D_D(DFileManagerWindow);

    d->viewStackLayout->setCurrentWidget(d->fileView);
    d->fileView->setViewModeToIcon();
}

void DFileManagerWindow::setListView()
{
    D_D(DFileManagerWindow);

    d->viewStackLayout->setCurrentWidget(d->fileView);
    d->fileView->setViewModeToList();
}

void DFileManagerWindow::preHandleCd(const FMEvent &event)
{
    D_DC(DFileManagerWindow);

    if (event.windowId() != windowId()) {
        return;
    }

    if (event.fileUrl().isNetWorkFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
    } else if (event.fileUrl().isSMBFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
    } else if (event.fileUrl().isComputerFile()) {
        showComputerView(event);
    } else if (event.fileUrl().isSearchFile()
               && d->viewStackLayout->currentWidget() == d->computerView) {
        return;
    } else {
        cd(event);
    }
}

void DFileManagerWindow::cd(const FMEvent &event)
{
    D_D(DFileManagerWindow);

    if (d->viewStackLayout->currentWidget() != d->fileView) {
        d->viewStackLayout->setCurrentWidget(d->fileView);
    }

    d->fileView->cd(event);
    d->toolbar->setViewModeButtonVisible(true);
}

void DFileManagerWindow::showComputerView(const FMEvent &event)
{
    D_D(DFileManagerWindow);

    d->viewStackLayout->setCurrentWidget(d->computerView);
    emit fileSignalManager->currentUrlChanged(event);
    d->toolbar->setViewModeButtonVisible(false);
}

void DFileManagerWindow::openNewTab(const FMEvent &event)
{
    D_D(DFileManagerWindow);

    if (event.windowId() != windowId()){
        return;
    }

    createNewView(event);

    if (d->views.count() >= 3){
        d->newTabButton->show();
    }
}

void DFileManagerWindow::createNewView(const FMEvent &event)
{
    D_D(DFileManagerWindow);

    DFileView* view = new DFileView(this);
    view->setObjectName("FileView");
    setFocusProxy(view);
    d->views.insert(event.fileUrl(), view);

    int viewIndex = d->viewStackLayout->addWidget(view);
    d->toolbar->addHistoryStack();
    d->toolbar->switchHistoryStack(d->toolbar->navStackCount()-1,event.fileUrl());

    d->viewStackLayout->setCurrentWidget(view);

    view->cd(event);
    d->fileView = view;

    connect(view, &DFileView::viewModeChanged, d->toolbar, &DToolBar::checkViewModeButton);
    connect(view, &DFileView::currentUrlChanged ,this, &DFileManagerWindow::onFileViewCurrentUrlChanged);

    DUrl url;
    if (event.fileUrl().isEmpty())
        url = DUrl::fromUserInput(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0));
    else
        url = event.fileUrl();


    const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);
    QString urlDisplayName = fileInfo->displayName();

    d->tabBar->addTabWithData(viewIndex,urlDisplayName,fileInfo->path());
    d->tabBar->setCurrentIndex(d->tabBar->count()-1);
    connect(view, &DFileView::viewModeChanged, d->toolbar, &DToolBar::checkViewModeButton);

    if (!d->tabBar->isHidden())
        d->newTabButton->show();

    d->leftSideBar->setFileView(d->fileView);
    d->leftSideBar->setToolbar(d->toolbar);
}

void DFileManagerWindow::onFileViewCurrentUrlChanged(const DUrl &url)
{
    D_D(DFileManagerWindow);

    int viewIndex = d->viewStackLayout->indexOf(d->fileView);
    const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);
    QString urlDisplayName = fileInfo->displayName();

    d->tabBar->setTabText(viewIndex,urlDisplayName, url.path());
}

void DFileManagerWindow::switchToView(const int viewIndex)
{
    D_D(DFileManagerWindow);

    d->viewStackLayout->setCurrentIndex(viewIndex);
    d->fileView = qobject_cast<DFileView*>(d->viewStackLayout->widget(viewIndex));
    d->leftSideBar->scene()->setCurrentUrl(d->fileView->currentUrl());
}

void DFileManagerWindow::moveCenter(const QPoint &cp)
{
    QRect qr = frameGeometry();

    qr.moveCenter(cp);
    move(qr.topLeft());
}

void DFileManagerWindow::moveTopRight()
{
    QRect pRect;
    pRect = qApp->desktop()->availableGeometry();
    int x = pRect.width() - width();
    move(QPoint(x, 0));
}

void DFileManagerWindow::moveTopRightByRect(QRect rect)
{
    int x = rect.x() + rect.width() - width();
    move(QPoint(x, 0));
}

void DFileManagerWindow::closeEvent(QCloseEvent *event)
{
    emit aboutToClose();
    DMainWindow::closeEvent(event);
}

void DFileManagerWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    D_DC(DFileManagerWindow);

    if (event->y() <= d->titleFrame->height()) {
        if (isMaximized())
            showNormal();
        else
            showMaximized();
    } else {
        DMainWindow::mouseDoubleClickEvent(event);
    }
}

void DFileManagerWindow::moveCenterByRect(QRect rect)
{
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}
