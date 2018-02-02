/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "dfmviewmanager.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "deviceinfo/udisklistener.h"
#include "usershare/usersharemanager.h"
#include "controllers/pathmanager.h"
#include "shutil/fileutils.h"
#include "interfaces/dfmsetting.h"
#include "gvfs/networkmanager.h"
#include "dde-file-manager/singleapplication.h"
#include "shutil/viewstatesmanager.h"

#include "xutil.h"
#include "utils.h"

#include "drenamebar.h"
#include "singleton.h"
#include "dfileservices.h"
#include "controllers/appcontroller.h"
#include "view/viewinterface.h"
#include "plugins/pluginmanager.h"
#include "controllers/trashmanager.h"
#include "themeconfig.h"
#include "controllers/fmstatemanager.h"

#include <dplatformwindowhandle.h>
#include <DThemeManager>
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
#include <QPair>


DWIDGET_USE_NAMESPACE

std::unique_ptr<RecordRenameBarState>  DFileManagerWindow::renameBarState{ nullptr };
std::atomic<bool> DFileManagerWindow::flagForNewWindowFromTab{ false };

class DFileManagerWindowPrivate
{
public:
    DFileManagerWindowPrivate(DFileManagerWindow *qq)
        : q_ptr(qq) {}

    void setCurrentView(DFMBaseView *view);
    bool processKeyPressEvent(QKeyEvent *event);

    QPushButton* logoButton{ nullptr };
    QFrame* centralWidget{ nullptr };
    DLeftSideBar* leftSideBar{ nullptr };
    QFrame* rightView { nullptr };
    DToolBar* toolbar{ nullptr };
    TabBar* tabBar { nullptr };
    QPushButton *newTabButton;
    DFMBaseView *currentView { nullptr };
    DStatusBar* statusBar { nullptr };
    QVBoxLayout* mainLayout { nullptr };
    DSplitter* splitter { nullptr };
    QFrame * titleFrame { nullptr };
    QStackedLayout* viewStackLayout { nullptr };
    QPushButton* emptyTrashButton { nullptr };
    DRenameBar* renameBar{ nullptr };

    QMap<DUrl, QWidget*> views;

    DFileManagerWindow *q_ptr{ nullptr };

    D_DECLARE_PUBLIC(DFileManagerWindow)
};

void DFileManagerWindowPrivate::setCurrentView(DFMBaseView *view)
{
    Q_Q(DFileManagerWindow);

    if (currentView && currentView->widget())
        currentView->widget()->removeEventFilter(q);

    currentView = view;

    if (currentView && currentView->widget())
        currentView->widget()->installEventFilter(q);

    if (!view)
        return;

    toolbar->setCustomActionList(view->toolBarActionList());

    if (!tabBar->currentTab()) {
        toolbar->addHistoryStack();
        tabBar->createTab(view);
    } else {
        tabBar->currentTab()->setFileView(view);
    }
}

bool DFileManagerWindowPrivate::processKeyPressEvent(QKeyEvent *event)
{
    Q_Q(DFileManagerWindow);

    switch (event->modifiers()) {
    case Qt::NoModifier: {
        switch (event->key()) {
        case Qt::Key_F1:
            appController->actionHelp();
            return true;
        case Qt::Key_F5:
            if (currentView)
                currentView->refresh();
            return true;
        }
        break;
    }
    case Qt::ControlModifier: {
        switch (event->key()) {
        case Qt::Key_Tab:
            tabBar->activateNextTab();
            return true;
        case Qt::Key_Backtab:
            tabBar->activatePreviousTab();
            return true;
        case Qt::Key_F:
            appController->actionctrlF(q->windowId());
            return true;
        case Qt::Key_L:
            appController->actionctrlL(q->windowId());
            return true;
        case Qt::Key_Left:
            appController->actionBack(q->windowId());
            return true;
        case Qt::Key_Right:
            appController->actionForward(q->windowId());
            return true;
        case Qt::Key_W:
            emit fileSignalManager->requestCloseCurrentTab(q->windowId());
            return true;
        }
        break;
    }
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_8) {
            tabBar->setCurrentIndex(event->key() - Qt::Key_1);
            return true;
        }

        switch (event->key()) {
        case Qt::Key_Left:
            appController->actionBack(q->windowId());
            return true;
        case Qt::Key_Right:
            appController->actionForward(q->windowId());
            return true;
        }

        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_Question) {
            appController->actionShowHotkeyHelp(q->windowId());
            return true;
        } else if (event->key() == Qt::Key_Tab) {
            tabBar->activatePreviousTab();
            return true;
        }
        break;
    }

    return false;
}

DFileManagerWindow::DFileManagerWindow(QWidget *parent)
    : DFileManagerWindow(DUrl(), parent)
{
}

DFileManagerWindow::DFileManagerWindow(const DUrl &fileUrl, QWidget *parent)
    : DMainWindow(parent)
    , d_ptr(new DFileManagerWindowPrivate(this))
{
    if (!DFMGlobal::IsFileManagerDiloagProcess){
        QString currentTheme = WindowManager::instance()->getFmStateManager()->fmState()->theme();
        DThemeManager::instance()->setTheme(this , currentTheme);
        ThemeConfig::instace()->update(currentTheme);
    }

    if (DFMGlobal::isRootUser()) {
        D_THEME_INIT_WIDGET(DFileManagerWindowAdmin)
    } else {
        D_THEME_INIT_WIDGET(DFileManagerWindow)
    }

    /// init global AppController
//    Q_UNUSED(AppController::instance());
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    titlebar()->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint| Qt::WindowSystemMenuHint);

    setWindowIcon(QIcon(":/images/images/dde-file-manager.svg"));

    initData();
    initUI();
    initConnect();

    openNewTab(fileUrl);
}

DFileManagerWindow::~DFileManagerWindow()
{
    m_currentTab = nullptr;
}

void DFileManagerWindow::onRequestCloseTab(const int index, const bool &remainState)
{
    D_D(DFileManagerWindow);

    Tab * tab = d->tabBar->tabAt(index);

    if (!tab)
        return;

    DFMBaseView *view = tab->fileView();

    d->viewStackLayout->removeWidget(view->widget());
    view->deleteLater();

    d->toolbar->removeNavStackAt(index);
    d->tabBar->removeTab(index, remainState);
}

void DFileManagerWindow::closeCurrentTab(quint64 winId)
{
    D_D(DFileManagerWindow);

    if (winId != this->winId())
        return;

    if (d->tabBar->count() == 1) {
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

void DFileManagerWindow::showEmptyTrashButton()
{
    Q_D(DFileManagerWindow);
    d->emptyTrashButton->show();
}

void DFileManagerWindow::hideEmptyTrashButton()
{
    Q_D(DFileManagerWindow);
    d->emptyTrashButton->hide();
}

void DFileManagerWindow::onNewTabButtonClicked()
{
    DUrl url;
    const QString& path = globalSetting->newTabPath();
    if(path == "Current Path")
        url = currentUrl();
    else
        url = DUrl::fromUserInput(path);

    openNewTab(url);
}

void DFileManagerWindow::requestEmptyTrashFiles()
{
    DFMGlobal::clearTrash();
}

void DFileManagerWindow::onTrashStateChanged()
{
    Q_D(DFileManagerWindow);
    if(currentUrl() == DUrl::fromTrashFile("/") && !TrashManager::isEmpty()){
        showEmptyTrashButton();
    } else{
        hideEmptyTrashButton();
    }
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

    if (tab) {
        d->toolbar->switchHistoryStack(tabIndex);

        if (!tab->fileView())
            return;

        d->toolbar->setCrumb(tab->fileView()->rootUrl());
        switchToView(tab->fileView());

        if (currentUrl().isSearchFile()) {
            if(!d->toolbar->getSearchBar()->isVisible()) {
                d->toolbar->searchBarActivated();
                d->toolbar->getSearchBar()->setText(tab->fileView()->rootUrl().searchKeyword());
            }
        } else {
            if(d->toolbar->getSearchBar()->isVisible()) {
                d->toolbar->searchBarDeactivated();
            }
        }
    }
}

DUrl DFileManagerWindow::currentUrl() const
{
    D_DC(DFileManagerWindow);

    return d->currentView ? d->currentView->rootUrl() : DUrl();
}

bool DFileManagerWindow::isCurrentUrlSupportSearch(const DUrl &currentUrl)
{
    const DAbstractFileInfoPointer &currentFileInfo = DFileService::instance()->createFileInfo(this, currentUrl);

    if (!currentFileInfo || !currentFileInfo->canIteratorDir())
        return false;
    return true;
}

DToolBar *DFileManagerWindow::getToolBar() const
{
    D_DC(DFileManagerWindow);

    return d->toolbar;
}

DFMBaseView *DFileManagerWindow::getFileView() const
{
    D_DC(DFileManagerWindow);

    return d->currentView;
}

DLeftSideBar *DFileManagerWindow::getLeftSideBar() const
{
    D_DC(DFileManagerWindow);

    return d->leftSideBar;
}

quint64 DFileManagerWindow::windowId()
{
    return WindowManager::getWindowId(this);
}

bool DFileManagerWindow::tabAddable() const
{
    D_DC(DFileManagerWindow);
    return d->tabBar->tabAddable();
}

bool DFileManagerWindow::cd(const DUrl &fileUrl, bool canFetchNetwork)
{
    D_D(DFileManagerWindow);

    if (currentUrl() == fileUrl)
        return true;

    if (canFetchNetwork && NetworkManager::SupportScheme.contains(fileUrl.scheme())) {
        emit fileSignalManager->requestFetchNetworks(DFMUrlBaseEvent(this, fileUrl));
        return true;
    }

    if (fileUrl.scheme() == "mount") {
        DUrl newUrl;
        QUrlQuery query(fileUrl);

        newUrl.setQuery(query.queryItemValue("id"));

        appController->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(this, newUrl));
        return true;
    }

    if (!d->currentView || !DFMViewManager::instance()->isSuited(fileUrl, d->currentView)) {
        DFMBaseView *view = DFMViewManager::instance()->createViewByUrl(fileUrl);

        if (view) {
            d->viewStackLayout->addWidget(view->widget());
            d->viewStackLayout->setCurrentWidget(view->widget());

            handleNewView(view);
        } else {
            qWarning() << "Not support url: " << fileUrl;

            //###(zccrs):
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, fileUrl);

            if (fileInfo) {
                /* Call fileInfo->exists() twice. First result is false and the second one is true;
                           Maybe this is a bug of fuse when smb://10.0.10.30/people is mounted and cd to mounted folder immediately.
                        */
                qDebug() << fileInfo->exists() << fileUrl;
                qDebug() << fileInfo->exists() << fileUrl;
            }

            if (!fileInfo || !fileInfo->exists()) {
                if (!isCurrentUrlSupportSearch(currentUrl()))
                    return false;

                const DUrl &newUrl = DUrl::fromSearchFile(currentUrl(), fileUrl.toString());
                const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, newUrl);

                if (!fileInfo || !fileInfo->exists())
                    return false;

                return cd(newUrl, canFetchNetwork);
            }

            return false;
        }

        if (d->currentView)
            d->currentView->deleteLater();

        d->setCurrentView(view);
    }

    bool ok = false;

    if (d->currentView)
       ok = d->currentView->setRootUrl(fileUrl);

    emit currentUrlChanged();
    this->hideRenameBar();

    return ok;
}

bool DFileManagerWindow::openNewTab(DUrl fileUrl)
{
    D_D(DFileManagerWindow);

    if (!d->tabBar->tabAddable())
        return false;

    if (fileUrl.isEmpty())
        fileUrl = DUrl::fromLocalFile(QDir::homePath());

    d->toolbar->addHistoryStack();
    d->setCurrentView(0);
    d->tabBar->createTab(Q_NULLPTR);

    return cd(fileUrl);
}

void DFileManagerWindow::switchToView(DFMBaseView *view)
{
    D_D(DFileManagerWindow);

    if (d->currentView == view)
        return;

    const DUrl &old_url = currentUrl();

    d->setCurrentView(view);
    d->viewStackLayout->setCurrentWidget(view->widget());

    if (view && view->rootUrl() == old_url) {
        return;
    }

    emit currentUrlChanged();
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

void DFileManagerWindow::keyPressEvent(QKeyEvent *event)
{
    Q_D(DFileManagerWindow);

    if (!d->processKeyPressEvent(event))
        return DMainWindow::keyPressEvent(event);
}

bool DFileManagerWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (!getFileView() || watched != getFileView()->widget())
        return false;

    if (event->type() != QEvent::KeyPress)
        return false;

    Q_D(DFileManagerWindow);

    return d->processKeyPressEvent(static_cast<QKeyEvent*>(event));
}

void DFileManagerWindow::resizeEvent(QResizeEvent *event)
{
    Q_D(DFileManagerWindow);
    DMainWindow::resizeEvent(event);
    d->titleFrame->setFixedSize(event->size().width() - titlebar()->buttonAreaWidth(), TITLE_FIXED_HEIGHT);
}

bool DFileManagerWindow::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
{
    Q_UNUSED(resultData)
    Q_D(DFileManagerWindow);

    switch (event->type()) {
    case DFMEvent::Back:
        d->toolbar->back();
        return true;
    case DFMEvent::Forward:
        d->toolbar->forward();
        return true;
    case DFMEvent::OpenNewTab:
        if (event->windowId() != this->internalWinId())
            return false;

        openNewTab(event.staticCast<DFMUrlBaseEvent>()->url());

        return true;
    default: break;
    }

    return false;
}

QObject *DFileManagerWindow::object() const
{
    return const_cast<DFileManagerWindow*>(this);
}

void DFileManagerWindow::handleNewView(DFMBaseView *view)
{
    Q_UNUSED(view)
}

void DFileManagerWindow::initData()
{

}

void DFileManagerWindow::initUI()
{
    D_DC(DFileManagerWindow);

    resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    setMinimumSize(650, 420);
    initTitleBar();
    initCentralWidget();
    setCentralWidget(d->centralWidget);
}

void DFileManagerWindow::initTitleFrame()
{
    D_D(DFileManagerWindow);

    d->logoButton = new QPushButton("");
    d->logoButton->setObjectName("LogoButton");
    d->logoButton->setFixedSize(QSize(24, 24));
    d->logoButton->setFocusPolicy(Qt::NoFocus);

    initToolBar();

    d->titleFrame = new QFrame;
    d->titleFrame->setObjectName("TitleBar");
    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->addSpacing(12);
    titleLayout->addWidget(d->logoButton);
    titleLayout->addSpacing(12);
    titleLayout->addWidget(d->toolbar);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    d->titleFrame->setLayout(titleLayout);
    d->titleFrame->setFixedHeight(TITLE_FIXED_HEIGHT);
}

void DFileManagerWindow::initTitleBar()
{
    D_D(DFileManagerWindow);

    initTitleFrame();

    DFileMenu* menu = fileMenuManger->createToolBarSettingsMenu();

    menu->setProperty("DFileManagerWindow", (quintptr)this);
    menu->setProperty("ToolBarSettingsMenu", true);
    menu->setEventData(DUrl(), DUrlList() << DUrl(), winId(), this);

    QAction *set_theme_action = menu->actionAt(1);
    if (set_theme_action) {
        set_theme_action->setCheckable(true);
        set_theme_action->setText(tr("Dark Theme"));
        if (DThemeManager::instance()->theme(this) == "dark") {
            set_theme_action->setChecked(true);
        }
        connect(set_theme_action, &QAction::triggered, this, &DFileManagerWindow::onThemeChanged);
    }

    bool isDXcbPlatform = false;
    SingleApplication* app = static_cast<SingleApplication*>(qApp);
    if (app){
        isDXcbPlatform = app->isDXcbPlatform();
    }

    if (isDXcbPlatform){
        d->toolbar->getSettingsButton()->hide();
        titlebar()->setMenu(menu);
        titlebar()->setContentsMargins(0, 1, -1, 0);

        titlebar()->setCustomWidget(d->titleFrame, Qt::AlignLeft);
    }else{
       d->toolbar->getSettingsButton()->setMenu(menu);
    }
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

    QObject::connect(d->leftSideBar, &DLeftSideBar::moveSplitter, d->splitter, &DSplitter::moveSplitter);
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
    d->renameBar = new DRenameBar;

    this->initRenameBarState();

    d->emptyTrashButton = new QPushButton{ this };
    d->emptyTrashButton->setFixedHeight(25);
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
    mainLayout->addWidget(d->renameBar);
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

void DFileManagerWindow::initCentralWidget()
{
    D_D(DFileManagerWindow);
    initSplitter();

    d->centralWidget = new QFrame(this);
    d->centralWidget->setObjectName("CentralWidget");
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(d->splitter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    d->centralWidget->setLayout(mainLayout);
}

void DFileManagerWindow::initConnect()
{
    D_D(DFileManagerWindow);

    if (titlebar()) {
        QObject::connect(titlebar(), SIGNAL(minimumClicked()), parentWidget(), SLOT(showMinimized()));
        QObject::connect(titlebar(), SIGNAL(maximumClicked()), parentWidget(), SLOT(showMaximized()));
        QObject::connect(titlebar(), SIGNAL(restoreClicked()), parentWidget(), SLOT(showNormal()));
        QObject::connect(titlebar(), SIGNAL(closeClicked()), parentWidget(), SLOT(close()));
    }

    QObject::connect(fileSignalManager, &FileSignalManager::fetchNetworksSuccessed, this, [this] (const DFMUrlBaseEvent &event) {
        if (event.windowId() != windowId())
            return;

        cd(event.fileUrl(), false);
    });
    QObject::connect(fileSignalManager, &FileSignalManager::requestCloseCurrentTab, this, &DFileManagerWindow::closeCurrentTab);

    QObject::connect(d->tabBar, &TabBar::tabMoved, d->toolbar, &DToolBar::moveNavStacks);
    QObject::connect(d->tabBar, &TabBar::currentChanged,this, &DFileManagerWindow::onCurrentTabChanged);
    QObject::connect(d->tabBar, &TabBar::tabCloseRequested, this,&DFileManagerWindow::onRequestCloseTab);
    QObject::connect(d->tabBar, &TabBar::tabAddableChanged, this, &DFileManagerWindow::onTabAddableChanged);

    QObject::connect(d->tabBar, &TabBar::tabBarShown, this, &DFileManagerWindow::showNewTabButton);
    QObject::connect(d->tabBar, &TabBar::tabBarHidden, this, &DFileManagerWindow::hideNewTabButton);
    QObject::connect(d->newTabButton, &QPushButton::clicked, this, &DFileManagerWindow::onNewTabButtonClicked);

    QObject::connect(d->emptyTrashButton, &QPushButton::clicked, this, &DFileManagerWindow::requestEmptyTrashFiles);

    QObject::connect(fileSignalManager, &FileSignalManager::trashStateChanged, this, &DFileManagerWindow::onTrashStateChanged);
    QObject::connect(fileSignalManager, &FileSignalManager::currentUrlChanged, this, &DFileManagerWindow::onTrashStateChanged);
    QObject::connect(fileSignalManager, &FileSignalManager::currentUrlChanged, d->tabBar, &TabBar::onCurrentUrlChanged);
    QObject::connect(d->tabBar, &TabBar::currentChanged, this, &DFileManagerWindow::onTrashStateChanged);

    QObject::connect(this, &DFileManagerWindow::currentUrlChanged, this, [this] {
        emit fileSignalManager->currentUrlChanged(DFMUrlBaseEvent(this, currentUrl()));
    });

    QObject::connect(d->renameBar, &DRenameBar::clickCancelButton, this, &DFileManagerWindow::hideRenameBar);
    QObject::connect(fileSignalManager, &FileSignalManager::requestMultiFilesRename, this, &DFileManagerWindow::onShowRenameBar);
    QObject::connect(d->tabBar, &TabBar::currentChanged, this, &DFileManagerWindow::onTabBarCurrentIndexChange);
}

void DFileManagerWindow::moveCenterByRect(QRect rect)
{
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}


void DFileManagerWindow::onShowRenameBar(const DFMUrlListBaseEvent& event) noexcept
{
   DFileManagerWindowPrivate* const d { d_func() };

   if(event.windowId() == this->windowId()){
       d->renameBar->storeUrlList(event.urlList()); //### get the urls of selection.

       m_currentTab = d->tabBar->currentTab();
       d->renameBar->setVisible(true);
   }
}

void DFileManagerWindow::onTabBarCurrentIndexChange(const int &index)noexcept
{
    DFileManagerWindowPrivate* const d{ d_func() };

    if(m_currentTab != d->tabBar->tabAt(index)){

        if(d->renameBar->isVisible() == true){
            if(d->renameBar->isVisible() == true){
                this->onReuqestCacheRenameBarState();//###: invoke this function before setVisible.

                d->renameBar->setVisible(false);
                d->renameBar->restoreRenameBar(); //###: when after hiding RenameBar, Must restore RenameBar.
            }
        }

     }
}

void DFileManagerWindow::hideRenameBar() noexcept //###: Hide renamebar and then clear history.
{
    DFileManagerWindowPrivate* const d{ d_func() };
    d->renameBar->setVisible(false);
    d->renameBar->restoreRenameBar();
}


void DFileManagerWindow::onReuqestCacheRenameBarState()const
{
    const DFileManagerWindowPrivate* const d{ d_func() };
    DFileManagerWindow::renameBarState = d->renameBar->getCurrentState();//###: record current state, when a new window is created from a already has tab.
}

void DFileManagerWindow::setTheme(const QString &theme)
{
    DThemeManager::instance()->setTheme(this , theme);

    DFileMenu* dfmenu = static_cast<DFileMenu *>(titlebar()->menu());
    QAction* theme_action = dfmenu->actionAt(1);
    if (theme_action){
        if (theme == "dark") {
            theme_action->setChecked(true);
        }
    }
}

void DFileManagerWindow::onThemeChanged()
{
    QString theme = DThemeManager::instance()->theme(this);
    if (theme == "light") {
        theme = "dark";
    } else {
        theme = "light";
    }
    foreach (const QWidget* w , WindowManager::instance()->getWindows().keys()) {
        const DFileManagerWindow* dfm = static_cast<const DFileManagerWindow *>(w);
        const_cast<DFileManagerWindow *>(dfm)->setTheme(theme);
    }

    WindowManager::instance()->saveWindowState(this);
}

void DFileManagerWindow::initRenameBarState()
{
    DFileManagerWindowPrivate* const d{ d_func() };

    bool expected{ true };
    ///###: CAS, when we draged a tab to leave TabBar for creating a new window.
    if(DFileManagerWindow::flagForNewWindowFromTab.compare_exchange_strong(expected, false, std::memory_order_seq_cst)){

        if(static_cast<bool>(DFileManagerWindow::renameBarState) == true){ //###: when we drag a tab to create a new window, but the RenameBar is showing in last window.
             d->renameBar->loadState(DFileManagerWindow::renameBarState);

        }else{  //###: when we drag a tab to create a new window, but the RenameBar is hiding.
             d->renameBar->setVisible(false);
        }

    }else{ //###: when open a new window from right click menu.
         d->renameBar->setVisible(false);
    }
}


void DFileManagerWindow::requestToSelectUrls()
{
    DFileManagerWindowPrivate* const d{ d_func() };
    if(static_cast<bool>(DFileManagerWindow::renameBarState) == true){
        d->renameBar->loadState(DFileManagerWindow::renameBarState);

        QList<DUrl> selectedUrls{ DFileManagerWindow::renameBarState->getSelectedUrl() };
        quint64 winId{ this->windowId() };
        DFMUrlListBaseEvent event{ nullptr,  selectedUrls};
        event.setWindowId(winId);

        QTimer::singleShot(100, [=]{ emit fileSignalManager->requestSelectFile(event); });

        DFileManagerWindow::renameBarState.reset(nullptr);
    }
}
