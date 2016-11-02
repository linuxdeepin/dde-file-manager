#include "dfilemanagerwindow.h"
#include "dleftsidebar.h"
#include "dtoolbar.h"
#include "dfileview.h"
#include "fileviewhelper.h"
#include "ddetailview.h"
#include "dfilemenu.h"
#include "dsearchbar.h"
#include "dsplitter.h"
#include "extendview.h"
#include "dstatusbar.h"
#include "dfilemenumanager.h"
#include "computerview.h"
#include "dtabbar.h"
#include "dbookmarkscene.h"
#include "windowmanager.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "deviceinfo/udisklistener.h"
#include "usershare/usersharemanager.h"
#include "controllers/pathmanager.h"

#include "xutil.h"
#include "utils.h"

#include "widgets/singleton.h"
#include "dfileservices.h"
#include "controllers/appcontroller.h"

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
    QPushButton* logoButton = NULL;
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
    : DFileManagerWindow(DUrl(), parent)
{

}

DFileManagerWindow::DFileManagerWindow(const DUrl &fileUrl, QWidget *parent)
    : DMainWindow(parent)
    , d_ptr(new DFileManagerWindowPrivate(this))
{
    setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));

    initData();
    initUI();
    initConnect();
    initFileView(fileUrl);
}

DFileManagerWindow::~DFileManagerWindow()
{

}

void DFileManagerWindow::onCurrentTabClosed(const int index, const bool &remainState){

    D_D(DFileManagerWindow);

    int viewIndex = d->tabBar->tabData(index).toJsonObject()["viewIndex"].toInt();
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
    d->tabBar->removeTab(index, remainState);

    if(d->tabBar->count()<8)
        emit d->tabBar->tabAddableChanged(true);

    if(d->tabBar->count()<2)
        d->tabBar->hide();

    if(d->tabBar->isHidden())
        d->newTabButton->hide();
}

void DFileManagerWindow::closeCurrentTab(const DFMEvent &event)
{
    D_D(DFileManagerWindow);

    if(event.windowId() != (int)winId())
        return;

    if(d->tabBar->count() == 1){
        close();
        return;
    }

    emit d->tabBar->tabCloseRequested(d->tabBar->currentIndex());
}

void DFileManagerWindow::onUserShareCountChanged(const int &count)
{
   D_D(DFileManagerWindow);
   if (d->fileView->rootUrl() == DUrl::fromUserShareFile("/")){
       d->fileView->fileViewHelper()->onUserShareCountChanged(count);
   }

}

void DFileManagerWindow::onFileDeleted(const DUrl &url)
{
    Q_D(const DFileManagerWindow);

    const DUrl &currentUrl = this->currentUrl();

    if (url != currentUrl) {
        const DAbstractFileInfoPointer &fileInfo = d->fileView->model()->fileInfo(currentUrl);

        if (!fileInfo || !fileInfo->isAncestorsUrl(url))
            return;
    }

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(url);
    const DUrl &parentUrl = fileInfo->parentUrl();

    DFMEvent event;

    event << WindowManager::getWindowId(this);
    event << DFMEvent::FileView;
    event << (parentUrl.isValid() ? parentUrl : DUrl::fromLocalFile(QDir::homePath()));

    preHandleCd(event);
}

QString DFileManagerWindow::getDisplayNameByUrl(const DUrl &url) const
{
    QString urlDisplayName;
    UDiskDeviceInfo* info = deviceListener->getDeviceByPath(url.toLocalFile());

    if (info) {
        urlDisplayName = info->fileDisplayName();
    } else {
        if(url.isComputerFile()){
            if(systemPathManager->isSystemPath(url.toString()))
                urlDisplayName = systemPathManager->getSystemPathDisplayNameByPath(url.toString());
        } else{
            const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);

            if (fileInfo)
                urlDisplayName = fileInfo->fileDisplayName();
        }
    }

    return urlDisplayName;
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
    DUrl url(d->tabBar->tabData(tabIndex).toJsonObject()["url"].toString());

    d->toolbar->switchHistoryStack(tabIndex,url);

    switchToView(viewIndex, url);
}


DUrl DFileManagerWindow::currentUrl() const
{
    D_DC(DFileManagerWindow);

    if (d->viewStackLayout->currentWidget() == d->fileView) {
        return d->fileView->rootUrl();
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

DLeftSideBar *DFileManagerWindow::getLeftSideBar() const
{
    D_DC(DFileManagerWindow);

    return d->leftSideBar;
}

int DFileManagerWindow::windowId()
{
    return WindowManager::getWindowId(this);
}

bool DFileManagerWindow::tabAddable() const
{
    D_DC(DFileManagerWindow);
    return d->tabBar->tabAddable();
}

void DFileManagerWindow::setFileViewMode(int viewMode)
{
    D_D(DFileManagerWindow);

    d->fileView->setViewMode(static_cast<DFileView::ViewMode>(viewMode));
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

void DFileManagerWindow::preHandleCd(const DFMEvent &event)
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
    } else {
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(event.fileUrl());

        if (!fileInfo || !fileInfo->exists()) {
            const DAbstractFileInfoPointer &currentFileInfo = DFileService::instance()->createFileInfo(currentUrl());

            if (!currentFileInfo || !currentFileInfo->canIteratorDir())
                return;

            const_cast<DFMEvent&>(event) << DUrl::fromSearchFile(currentUrl(), event.fileUrl().toString());

            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(event.fileUrl());

            if (!fileInfo || !fileInfo->exists())
                return;
        }

        cd(event);
    }
}

void DFileManagerWindow::cd(const DFMEvent &event)
{
    D_D(DFileManagerWindow);

    if (d->viewStackLayout->currentWidget() != d->fileView) {
        d->viewStackLayout->setCurrentWidget(d->fileView);
        emit fileSignalManager->currentUrlChanged(event);
        onFileViewCurrentUrlChanged(event.fileUrl());
    }

    d->fileView->fileViewHelper()->cd(event);
    d->toolbar->setViewModeButtonVisible(true);
}

void DFileManagerWindow::showComputerView(const DFMEvent &event)
{
    D_D(DFileManagerWindow);

    if (!d->computerView)
        initComputerView();

    d->viewStackLayout->setCurrentWidget(d->computerView);
    d->computerView->setFocus();
    emit fileSignalManager->currentUrlChanged(event);
    d->toolbar->setViewModeButtonVisible(false);
    onFileViewCurrentUrlChanged(DUrl::fromComputerFile("/"));
}

void DFileManagerWindow::openNewTab(const DFMEvent &event)
{
    D_D(DFileManagerWindow);

    if(!d->tabBar->tabAddable())
        return;

    if (event.windowId() != windowId()){
        return;
    }

    createNewView(event);

    if (d->views.count() >= 3){
        d->newTabButton->show();
    }
}

void DFileManagerWindow::createNewView(const DFMEvent &event)
{
    D_D(DFileManagerWindow);

    DFileView* view = new DFileView(this);
    view->setObjectName("FileView");
    setFocusProxy(view);
    d->views.insert(event.fileUrl(), view);

    int viewIndex = d->viewStackLayout->addWidget(view);
    d->toolbar->addHistoryStack();

    DUrl url;
    if (event.fileUrl().isEmpty())
        url = DUrl::fromUserInput(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0));
    else
        url = event.fileUrl();

    if(deviceListener->isDeviceFolder(url.path())){
        appController->actionOpenDisk(event);
    }

    QString urlDisplayName = getDisplayNameByUrl(url);

    setFileView(view);
    d->tabBar->addTabWithData(viewIndex,urlDisplayName,url);
    view->cd(url);
}

void DFileManagerWindow::setFileView(DFileView *view)
{
    D_D(DFileManagerWindow);

    d->viewStackLayout->setCurrentWidget(view);

    if(d->fileView){
        view->setViewMode(d->fileView->getDefaultViewMode());
        disconnect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, d->fileView->statusBar(), &DStatusBar::itemSelected);
        disconnect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, d->fileView->statusBar(), &DStatusBar::itemCounted);
        disconnect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, d->fileView->statusBar(), &DStatusBar::setLoadingIncatorVisible);
    }

    d->fileView = view;

    connect(view, &DFileView::viewModeChanged, d->toolbar, &DToolBar::checkViewModeButton);
    connect(view, &DFileView::rootUrlChanged ,this, &DFileManagerWindow::onFileViewCurrentUrlChanged);

    if (!d->tabBar->isHidden())
        d->newTabButton->show();

    d->leftSideBar->setToolbar(d->toolbar);
}

void DFileManagerWindow::onFileViewCurrentUrlChanged(const DUrl &url)
{
    D_D(DFileManagerWindow);

    if(!url.isValid())
        return;

    int viewIndex = d->viewStackLayout->indexOf(d->fileView);

    QString urlDisplayName = getDisplayNameByUrl(url);

    d->tabBar->setTabText(viewIndex,urlDisplayName, url);
}

void DFileManagerWindow::switchToView(const int viewIndex, const DUrl &url)
{
    D_D(DFileManagerWindow);

    if (d->fileView) {
        disconnect(d->fileView, &DFileView::viewModeChanged, d->toolbar, &DToolBar::checkViewModeButton);
        disconnect(d->fileView, &DFileView::rootUrlChanged ,this, &DFileManagerWindow::onFileViewCurrentUrlChanged);

        disconnect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, d->fileView->statusBar(), &DStatusBar::itemSelected);
        disconnect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, d->fileView->statusBar(), &DStatusBar::itemCounted);
        disconnect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, d->fileView->statusBar(), &DStatusBar::setLoadingIncatorVisible);
    }

//    d->viewStackLayout->setCurrentIndex(viewIndex);
    d->fileView = qobject_cast<DFileView*>(d->viewStackLayout->widget(viewIndex));
    if(url.isComputerFile()){
        if (!d->computerView)
            initComputerView();

        d->viewStackLayout->setCurrentWidget(d->computerView);
        d->computerView->setFocus();
        d->toolbar->setViewModeButtonVisible(false);
    } else {
        d->viewStackLayout->setCurrentWidget(d->fileView);
        d->toolbar->setViewModeButtonVisible(true);
    }
    if(!d->fileView)
        return;

    connect(d->fileView, &DFileView::viewModeChanged, d->toolbar, &DToolBar::checkViewModeButton);
    connect(d->fileView, &DFileView::rootUrlChanged ,this, &DFileManagerWindow::onFileViewCurrentUrlChanged);

    connect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, d->fileView->statusBar(), &DStatusBar::itemSelected);
    connect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, d->fileView->statusBar(), &DStatusBar::itemCounted);
    connect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, d->fileView->statusBar(), &DStatusBar::setLoadingIncatorVisible);


    emit fileViewChanged(d->fileView);
    d->leftSideBar->scene()->setCurrentUrl(url);
    d->toolbar->checkViewModeButton(d->fileView->viewMode());

    if (url == DUrl::fromUserShareFile("/")){
        qDebug() << "===========" << userShareManager->validShareInfoCount();
        if (userShareManager->validShareInfoCount() == 0){
            DFMEvent event;
            event << windowId();
            event << DUrl::fromLocalFile(QDir::homePath());
            emit fileSignalManager->requestChangeCurrentUrl(event);
        }
    }
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

void DFileManagerWindow::moveEvent(QMoveEvent *event)
{
    DMainWindow::moveEvent(event);

    emit positionChanged(event->pos());
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

void DFileManagerWindow::initTitleFrame()
{
    D_D(DFileManagerWindow);

    d->logoButton = new QPushButton("");
    d->logoButton->setObjectName("LogoButton");
    d->logoButton->setFixedSize(QSize(24, 24));
    d->logoButton->setFocusPolicy(Qt::NoFocus);

    initToolBar();
    initTitleBar();

    d->titleFrame = new QFrame;
    d->titleFrame->setObjectName("TitleBar");
    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->addSpacing(12);
    titleLayout->addWidget(d->logoButton);
    titleLayout->addSpacing(12);
    titleLayout->addWidget(d->toolbar);
    titleLayout->addWidget(titleBar());
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    d->titleFrame->setLayout(titleLayout);
    d->titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);
}

void DFileManagerWindow::initTitleBar()
{
    if (!titleBar())
        return;

    DFileMenu* menu = fileMenuManger->createToolBarSettingsMenu();

    DFMEvent event;
    event << windowId();
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

    initTabBar();
    initViewLayout();

    d->rightView = new QFrame;

    QHBoxLayout *tabBarLayout = new QHBoxLayout;
    tabBarLayout->setMargin(0);
    tabBarLayout->setSpacing(0);
    tabBarLayout->addWidget(d->tabBar);
    tabBarLayout->addWidget(d->newTabButton);


    QVBoxLayout* mainLayout = new QVBoxLayout;
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

void DFileManagerWindow::initFileView(const DUrl &fileUrl)
{
    DFMEvent event;
    event << fileUrl;
    event << windowId();
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
    initTitleFrame();
    initSplitter();

    d->centralWidget = new QFrame(this);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(d->titleFrame);
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
        connect(fileSignalManager, &FileSignalManager::requestCloseCurrentTab, this, &DFileManagerWindow::closeCurrentTab);

    connect(d->tabBar, &TabBar::tabMoved, d->toolbar, &DToolBar::moveNavStacks);
    connect(d->tabBar, &TabBar::currentChanged,this, &DFileManagerWindow::onCurrentTabChanged);
    connect(d->tabBar, &TabBar::tabCloseRequested, this,&DFileManagerWindow::onCurrentTabClosed);
    connect(d->tabBar, &TabBar::tabAddableChanged, this, &DFileManagerWindow::onTabAddableChanged);
    connect(d->newTabButton, &QPushButton::clicked, this, [=]{
        DFMEvent event;
        const DUrl url = DUrl::fromUserInput(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0));
        event << url;
        event << windowId();
        openNewTab(event);
    });
    connect(fileService, &DFileService::childrenRemoved, this, &DFileManagerWindow::onFileDeleted);

    connect(fileSignalManager, &FileSignalManager::userShareCountChanged,
            this, &DFileManagerWindow::onUserShareCountChanged);
}

void DFileManagerWindow::moveCenterByRect(QRect rect)
{
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}
