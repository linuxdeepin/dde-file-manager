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
#include "shutil/fileutils.h"
#include "interfaces/dfmsetting.h"

#include "xutil.h"
#include "utils.h"

#include "widgets/singleton.h"
#include "dfileservices.h"
#include "controllers/appcontroller.h"
#include "viewmanager.h"
#include "view/viewinterface.h"
#include "plugins/pluginmanager.h"
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

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

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
    QPushButton* emptyTrashButton = NULL;

    QMap<DUrl, QWidget*> views;
    ViewManager* viewManager = NULL;

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
    /// init global AppController
//    Q_UNUSED(AppController::instance());

    setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));

    initData();
    initUI();
    initConnect();

    if (d_ptr->viewManager->isSchemeRegistered(fileUrl.scheme())){
        const_cast<DUrl&>(fileUrl) = DUrl(fileUrl.scheme() + ":///");
    }
    initFileView(fileUrl);
}

DFileManagerWindow::~DFileManagerWindow()
{

}

void DFileManagerWindow::onRequestCloseTab(const int index, const bool &remainState){

    D_D(DFileManagerWindow);

    Tab * tab = d->tabBar->tabAt(index);
    if(!tab)
        return;
    DFileView *view = tab->fileView();

    d->viewStackLayout->removeWidget(view);
    d->viewManager->unRegisterView(view->viewId());

    d->toolbar->removeNavStackAt(index);
    d->tabBar->removeTab(index, remainState);
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

void DFileManagerWindow::showNewTabButton()
{
    D_D(DFileManagerWindow);
    d->newTabButton->show();
}

void DFileManagerWindow::hideNewTabButton()
{
    D_D(DFileManagerWindow);
    d->newTabButton->hide();
}

void DFileManagerWindow::onNewTabButtonClicked()
{
    DFMEvent event;
    DUrl url;
    const QString& path = globalSetting->newTabPath();
    if(path == "Current Path")
        url = currentUrl();
    else
        url = DUrl::fromUserInput(path);
    event << url;
    event << windowId();
    openNewTab(event);
}

void DFileManagerWindow::onFileViewSelectionChanged()
{
    D_D(DFileManagerWindow);

    if(currentUrl() == DUrl::fromTrashFile("/") && d->fileView->selectedIndexCount()>0){
        if(d->emptyTrashButton->isHidden())
            showEmptyButton();
    }
    else{
        if(!d->emptyTrashButton->isHidden())
            hideEmptyButton();
    }
}

void DFileManagerWindow::requestEmptyTrashFiles()
{
    D_D(DFileManagerWindow);
    DFMEvent event;
    event << windowId();
    event << currentUrl();
    event << d->fileView->selectedUrls();
    appController->actionCompleteDeletion(event);
}

void DFileManagerWindow::showEmptyButton()
{
    D_D(DFileManagerWindow);
    d->emptyTrashButton->show();
    QVariantAnimation* ani = new QVariantAnimation(this);
    ani->setStartValue(0);
    ani->setEndValue(25);
    ani->setDuration(240);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    connect(ani, &QVariantAnimation::valueChanged,[=] (const QVariant& val){
        d->emptyTrashButton->setFixedHeight(val.toInt());
    });
    connect(ani, &QVariantAnimation::finished, [=]{
        ani->deleteLater();
    });
    ani->start();
}

void DFileManagerWindow::hideEmptyButton()
{
    D_D(DFileManagerWindow);
    d->emptyTrashButton->show();
    QVariantAnimation* ani = new QVariantAnimation(this);
    ani->setStartValue(25);
    ani->setEndValue(0);
    ani->setDuration(240);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    connect(ani, &QVariantAnimation::valueChanged, [=] (const QVariant& val){
        d->emptyTrashButton->setFixedHeight(val.toInt());
    });
    connect(ani, &QVariantAnimation::finished, [=]{
        ani->deleteLater();
        d->emptyTrashButton->hide();
    });
    ani->start();
}

void DFileManagerWindow::onTabAddableChanged(bool addable)
{
    D_D(DFileManagerWindow);

    d->newTabButton->setEnabled(addable);
}
void DFileManagerWindow::onCurrentTabChanged(int tabIndex)
{
    D_D(DFileManagerWindow);

    Tab* tab = d->tabBar->tabAt(tabIndex);
    if(tab){
        d->toolbar->switchHistoryStack(tabIndex,tab->fileView()->rootUrl());
        switchToView(tab->fileView());
        if(currentUrl().isSearchFile()){
            if(!d->toolbar->getSearchBar()->isVisible()){
                d->toolbar->searchBarActivated();
                d->toolbar->getSearchBar()->setText(tab->fileView()->rootUrl().searchKeyword());
            }
        } else{
            if(d->toolbar->getSearchBar()->isVisible()){
                d->toolbar->searchBarDeactivated();
            }
        }
    }
}


DUrl DFileManagerWindow::currentUrl() const
{
    D_DC(DFileManagerWindow);

    if (d->viewStackLayout->currentWidget() == d->fileView) {
        return d->fileView->rootUrl();
    } else if (d->viewStackLayout->currentWidget() == d->computerView) {
        return d->computerView->rootUrl();
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

ViewManager *DFileManagerWindow::getViewManager() const
{
    D_DC(DFileManagerWindow);

    return d->viewManager;
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

void DFileManagerWindow::preHandleCd(const DUrl &fileUrl, int source)
{
    D_DC(DFileManagerWindow);
    qDebug() << fileUrl << source << d->viewManager->supportSchemes() << d->viewManager->views() << d->viewManager->isSchemeRegistered(fileUrl.scheme());
    DFMEvent event;

    event << fileUrl;
    event << (DFMEvent::EventSource)(source);
    event << this->windowId();

    if (event.fileUrl().isNetWorkFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
    }else if (event.fileUrl().isSMBFile()) {
        emit fileSignalManager->requestFetchNetworks(event);
    }else if (d->viewManager->isSchemeRegistered(fileUrl.scheme())){
        showPluginView(fileUrl);

        QString viewId = fileUrl.scheme() + ":///";
        event << DUrl(viewId);
        emit fileSignalManager->currentUrlChanged(event);

    } else if (event.fileUrl().isComputerFile()) {
        event << DUrl::fromComputerFile("/");
        d->tabBar->currentTab()->setCurrentUrl(event.fileUrl());
        emit d->tabBar->currentChanged(d->tabBar->currentIndex());
        d->toolbar->setCrumb(event.fileUrl());
    } else if (!fileUrl.toString().isEmpty()) {

        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(event.fileUrl());
        if(fileInfo){
            /*call fileInfo->exists() twice, Firsrly result is false;Secondly result is true;
              May be this is bug of fuse when smb://10.0.10.30/people is mounted and cd to mounted folder immediately.
            */
            qDebug() << fileInfo->exists() << fileUrl.toString();
            qDebug() << fileInfo->exists() << fileUrl.toString();
        }
        if (!fileInfo || !fileInfo->exists()) {
            if (!FileUtils::isFileExists(fileUrl.toString())){
                if (DUrl::hasScheme(event.fileUrl().scheme()))
                    return;

                const DAbstractFileInfoPointer &currentFileInfo = DFileService::instance()->createFileInfo(currentUrl());

                if (!currentFileInfo || !currentFileInfo->canIteratorDir())
                    return;

                const_cast<DFMEvent&>(event) << DUrl::fromSearchFile(currentUrl(), event.fileUrl().toString());

                const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(event.fileUrl());

                if (!fileInfo || !fileInfo->exists())
                    return;
            }
        }
        cd(event);
    }
}

void DFileManagerWindow::preHandleCd(const DFMEvent &event)
{
    if (event.windowId() != windowId()) {
        return;
    }

    preHandleCd(event.fileUrl(), event.source());
}

void DFileManagerWindow::cd(const DFMEvent &event)
{
    D_D(DFileManagerWindow);

    if (d->viewStackLayout->currentWidget() != d->fileView) {
        d->viewStackLayout->setCurrentWidget(d->fileView);
        emit fileSignalManager->currentUrlChanged(event);
    }

    d->fileView->fileViewHelper()->cd(event);
    d->toolbar->setViewModeButtonVisible(true);

    if(event.fileUrl() == DUrl::fromTrashFile("/") && d->fileView->selectedIndexCount()>0){
        if(d->emptyTrashButton->isHidden())
            showEmptyButton();
    } else{
        if(!d->emptyTrashButton->isHidden())
            hideEmptyButton();
    }
}

void DFileManagerWindow::showPluginView(const DUrl &fileUrl)
{
    D_D(DFileManagerWindow);
    QString scheme = fileUrl.scheme();
    QString viewId = scheme + "0";
    QWidget* view;
    if (!d->viewManager->isViewIdRegistered(viewId)){
        ViewInterface* viewInterface = PluginManager::instance()->getViewInterfaceByScheme(scheme);
        if (viewInterface){
            view = viewInterface->createView();
        }else if (scheme == COMPUTER_SCHEME){
            d->computerView = new ComputerView(this);
            view = d->computerView;
        }
        qDebug() << "Delay create view" << viewId << view;
        d->viewManager->registerView(viewId, view);
        d->viewStackLayout->addWidget(view);
    }else{
        view = d->viewManager->getViewById(viewId);
    }

    DUrl currentUrl(fileUrl.scheme() + ":///");

    d->viewStackLayout->setCurrentWidget(view);
    view->setFocus();
    d->tabBar->currentTab()->setCurrentUrl(currentUrl);
    d->toolbar->setViewModeButtonVisible(false);
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

    if(d->viewManager->isSchemeRegistered(event.fileUrl().scheme())){
        preHandleCd(event.fileUrl(), DFMEvent::Unknow);
    }
}

void DFileManagerWindow::createNewView(const DFMEvent &event)
{
    D_D(DFileManagerWindow);

    DFileView* view = new DFileView(this);
    view->setObjectName("FileView");
    setFocusProxy(view);

    d->viewStackLayout->addWidget(view);
    d->toolbar->addHistoryStack();

    DUrl fileUrl;
    if (event.fileUrl().isEmpty())
        fileUrl = DUrl::fromLocalFile(QDir::homePath());
    else
        fileUrl = event.fileUrl();

    d->tabBar->createTab(view);

    d->viewManager->registerView(view->viewId(), view);

    view->cd(fileUrl);
}

void DFileManagerWindow::switchToView(DFileView *view)
{
    D_D(DFileManagerWindow);

    if (d->fileView) {
        disconnect(d->fileView, &DFileView::viewModeChanged, d->toolbar, &DToolBar::checkViewModeButton);
        disconnect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, d->fileView->statusBar(), &DStatusBar::setLoadingIncatorVisible);
        disconnect(d->fileView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileManagerWindow::onFileViewSelectionChanged);
        disconnect(d->fileView, &DFileView::requestActivateTabByIndex, d->tabBar, &TabBar::setCurrentIndex);
        d->fileView->fileViewHelper()->setIsActive(false);
    }

    d->fileView = view;
    d->fileView->fileViewHelper()->setIsActive(true);

    DUrl currentUrl = d->tabBar->currentTab()->currentUrl();
    if(d->viewManager->isSchemeRegistered(currentUrl.scheme())){
        showPluginView(currentUrl);
        d->toolbar->setCrumb(currentUrl);
    } else {
        d->viewStackLayout->setCurrentWidget(d->fileView);
        d->toolbar->setViewModeButtonVisible(true);

        connect(d->fileView, &DFileView::viewModeChanged, d->toolbar, &DToolBar::checkViewModeButton);
        connect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, d->fileView->statusBar(), &DStatusBar::setLoadingIncatorVisible);
        connect(d->fileView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileManagerWindow::onFileViewSelectionChanged);
        connect(d->fileView, &DFileView::requestActivateTabByIndex, d->tabBar, &TabBar::setCurrentIndex);

        d->leftSideBar->scene()->setCurrentUrl(view->rootUrl());
        d->toolbar->checkViewModeButton(d->fileView->viewMode());
        view->updateStatusBar();
        if(d->fileView->rootUrl() == DUrl::fromTrashFile("/") && d->fileView->selectedIndexCount()>0){
            if(d->emptyTrashButton->isHidden())
                showEmptyButton();
        } else{
            if(!d->emptyTrashButton->isHidden())
                hideEmptyButton();
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
    emit fileSignalManager->requestQuitApplication();
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

void DFileManagerWindow::initViewManager()
{
    D_D(DFileManagerWindow);
    d->viewManager = new ViewManager(this);
    d->viewManager->registerScheme(ComputerView::scheme());
    loadPluginRegisteredSchemes();
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

    d->emptyTrashButton = new QPushButton(this);
    d->emptyTrashButton->setFixedHeight(0);
    d->emptyTrashButton->hide();
    d->emptyTrashButton->setContentsMargins(0,0,0,0);
    d->emptyTrashButton->setObjectName("EmptyTrashButton");

    QHBoxLayout *tabBarLayout = new QHBoxLayout;
    tabBarLayout->setMargin(0);
    tabBarLayout->setSpacing(0);
    tabBarLayout->addWidget(d->tabBar);
    tabBarLayout->addWidget(d->newTabButton);


    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(tabBarLayout);
    mainLayout->addWidget(d->emptyTrashButton);
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
    preHandleCd(fileUrl, DFMEvent::Unknow);
}

void DFileManagerWindow::initComputerView()
{
    D_D(DFileManagerWindow);

    d->computerView = new ComputerView(this);
    d->viewStackLayout->addWidget(d->computerView);
    d->viewManager->registerView(d->computerView->viewId(), d->computerView);
}

void DFileManagerWindow::loadPluginRegisteredSchemes()
{
    D_D(DFileManagerWindow);
    foreach (ViewInterface* viewInterface, PluginManager::instance()->getViewInterfaces()) {
        d->viewManager->registerScheme(viewInterface->scheme());
    }
}

void DFileManagerWindow::initCentralWidget()
{
    D_D(DFileManagerWindow);
    initTitleFrame();
    initSplitter();
    initViewManager();

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
    connect(fileSignalManager, &FileSignalManager::requestChangeCurrentUrl,
            this, static_cast<void (DFileManagerWindow::*)(const DFMEvent&)>(&DFileManagerWindow::preHandleCd));
    connect(fileSignalManager, &FileSignalManager::requestCloseCurrentTab, this, &DFileManagerWindow::closeCurrentTab);

    connect(d->tabBar, &TabBar::tabMoved, d->toolbar, &DToolBar::moveNavStacks);
    connect(d->tabBar, &TabBar::currentChanged,this, &DFileManagerWindow::onCurrentTabChanged);
    connect(d->tabBar, &TabBar::tabCloseRequested, this,&DFileManagerWindow::onRequestCloseTab);
    connect(d->tabBar, &TabBar::tabAddableChanged, this, &DFileManagerWindow::onTabAddableChanged);

    connect(d->tabBar, &TabBar::tabBarShown, this, &DFileManagerWindow::showNewTabButton);
    connect(d->tabBar, &TabBar::tabBarHidden, this, &DFileManagerWindow::hideNewTabButton);
    connect(d->newTabButton, &QPushButton::clicked, this, &DFileManagerWindow::onNewTabButtonClicked);

    connect(d->emptyTrashButton, &QPushButton::clicked, this, &DFileManagerWindow::requestEmptyTrashFiles);

}

void DFileManagerWindow::moveCenterByRect(QRect rect)
{
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}
