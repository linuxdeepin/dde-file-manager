/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
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
#include "dtoolbar.h"
#include "dfileview.h"
#include "fileviewhelper.h"
#include "ddetailview.h"
#include "dfilemenu.h"
#include "dfmsplitter.h"
#include "extendview.h"
#include "dstatusbar.h"
#include "dfilemenumanager.h"
#include "computerview.h"
#include "dtabbar.h"
#include "windowmanager.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"
#include "dfmviewmanager.h"
#include "dfmsidebar.h"
#include "dfmsidebaritem.h"
#include "dfmaddressbar.h"
#include "dfmsettings.h"
#include "dfmapplication.h"
#include "dfmstandardpaths.h"

#include "app/define.h"
#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "deviceinfo/udisklistener.h"
#include "usershare/usersharemanager.h"
#include "controllers/pathmanager.h"
#include "shutil/fileutils.h"
#include "gvfs/networkmanager.h"
#include "dde-file-manager/singleapplication.h"

#include "xutil.h"
#include "utils.h"
#include "dfmadvancesearchbar.h"

#include "drenamebar.h"
#include "singleton.h"
#include "dfileservices.h"
#include "controllers/appcontroller.h"
#include "view/viewinterface.h"
#include "plugins/pluginmanager.h"
#include "controllers/trashmanager.h"
#include "themeconfig.h"

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
#include <QtConcurrent>

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
    bool cdForTab(Tab *tab, const DUrl &fileUrl);
    void initAdvanceSearchBar();
    bool isAdvanceSearchBarVisible() const;
    void setAdvanceSearchBarVisible(bool visible);
    void initRenameBar();
    bool isRenameBarVisible() const;
    void setRenameBarVisible(bool visible);
    void resetRenameBar();
    void storeUrlListToRenameBar(const QList<DUrl>& list) noexcept;

    QPushButton *logoButton{ nullptr };
    QFrame *centralWidget{ nullptr };
    DFMSideBar *leftSideBar{ nullptr };
    QFrame *rightView { nullptr };
    QVBoxLayout *rightViewLayout { nullptr };
    DToolBar *toolbar{ nullptr };
    TabBar *tabBar { nullptr };
    QPushButton *newTabButton;
    DFMBaseView *currentView { nullptr };
    DStatusBar *statusBar { nullptr };
    QVBoxLayout *mainLayout { nullptr };
    DFMSplitter *splitter { nullptr };
    QFrame *titleFrame { nullptr };
    QStackedLayout *viewStackLayout { nullptr };
    QPushButton *emptyTrashButton { nullptr };
    DRenameBar *renameBar{ nullptr };
    DFMAdvanceSearchBar *advanceSearchBar = nullptr;

    QMap<DUrl, QWidget *> views;

    DFileManagerWindow *q_ptr{ nullptr };

    D_DECLARE_PUBLIC(DFileManagerWindow)
};

void DFileManagerWindowPrivate::setCurrentView(DFMBaseView *view)
{
    Q_Q(DFileManagerWindow);

    if (currentView && currentView->widget()) {
        currentView->widget()->removeEventFilter(q);
    }

    currentView = view;

    if (currentView && currentView->widget()) {
        currentView->widget()->installEventFilter(q);
    }

    if (!view) {
        return;
    }

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
            if (currentView) {
                currentView->refresh();
            }
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
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            toolbar->triggerActionByIndex(event->key() - Qt::Key_1);
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
        } else if (event->key() == Qt::Key_Backtab) {
            tabBar->activatePreviousTab();
            return true;
        }
        break;
    }

    return false;
}

bool DFileManagerWindowPrivate::cdForTab(Tab *tab, const DUrl &fileUrl)
{
    DFMBaseView *current_view = tab->fileView();

    if (current_view && current_view->rootUrl() == fileUrl) {
        return false;
    }

    // old mount scheme support... may get removed when no other place use this...
    if (fileUrl.scheme() == "mount") {
        DUrl newUrl;
        QUrlQuery query(fileUrl);

        if (query.hasQueryItem("id")) {
            newUrl.setQuery(query.queryItemValue("id"));
            appController->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(q_ptr, newUrl));

            return true;
        }
    }

    if (!current_view || !DFMViewManager::instance()->isSuited(fileUrl, current_view)) {
        DFMBaseView *view = DFMViewManager::instance()->createViewByUrl(fileUrl);

        if (view) {
            viewStackLayout->addWidget(view->widget());

            if (tab == tabBar->currentTab())
                viewStackLayout->setCurrentWidget(view->widget());

            q_ptr->handleNewView(view);
        } else {
            qWarning() << "Not support url: " << fileUrl;

            //###(zccrs):
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(q_ptr, fileUrl);

            if (fileInfo) {
                /* Call fileInfo->exists() twice. First result is false and the second one is true;
                           Maybe this is a bug of fuse when smb://10.0.10.30/people is mounted and cd to mounted folder immediately.
                        */
                qDebug() << fileInfo->exists() << fileUrl;
                qDebug() << fileInfo->exists() << fileUrl;
            }

            if (!fileInfo || !fileInfo->exists()) {
                DUrl searchUrl = current_view->rootUrl();

                if (searchUrl.isComputerFile()) {
                    searchUrl = DUrl::fromLocalFile("/");
                }

                if (searchUrl.isSearchFile()) {
                    searchUrl = searchUrl.searchTargetUrl();
                }

                if (!q_ptr->isCurrentUrlSupportSearch(searchUrl)) {
                    return false;
                }

                const DUrl &newUrl = DUrl::fromSearchFile(searchUrl, fileUrl.toString());
                const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(q_ptr, newUrl);

                if (!fileInfo || !fileInfo->exists()) {
                    return false;
                }

                return cdForTab(tab, newUrl);
            }

            return false;
        }

        if (current_view) {
            current_view->deleteLater();
        }

        tab->setFileView(view);

        if (tab == tabBar->currentTab())
            setCurrentView(view);

        current_view = view;
    }

    bool ok = false;

    if (current_view) {
        ok = current_view->setRootUrl(fileUrl);

        if (ok) {
            tab->onFileRootUrlChanged(fileUrl);

            if (tab == tabBar->currentTab()) {
                emit q_ptr->currentUrlChanged();
            }
        }
    }

    return ok;
}

void DFileManagerWindowPrivate::initAdvanceSearchBar()
{
    if (advanceSearchBar) return;

    Q_Q(DFileManagerWindow);

    // blumia: we add the DFMAdvanceSearchBar widget to layout so actually we shouldn't give it a parent here,
    //         but we need to apply the currect stylesheet to it so set a parent will do this job.
    //         feel free to replace it with a better way to apply stylesheet.
    advanceSearchBar = new DFMAdvanceSearchBar(q);

    initRenameBar(); // ensure we can use renameBar.

    Q_CHECK_PTR(rightViewLayout);

    int renameWidgetIndex = rightViewLayout->indexOf(renameBar);
    int advanceSearchBarInsertTo = renameWidgetIndex == -1 ? 0 : renameWidgetIndex + 1;
    rightViewLayout->insertWidget(advanceSearchBarInsertTo, advanceSearchBar);

    QObject::connect(advanceSearchBar, &DFMAdvanceSearchBar::optionChanged, q, [ = ](const QMap<int, QVariant> &formData) {
        if (currentView) {
            DFileView *fv = dynamic_cast<DFileView*>(currentView);
            if (fv) {
                fv->setAdvanceSearchFilter(formData);
            }
        }
    });
}

bool DFileManagerWindowPrivate::isAdvanceSearchBarVisible() const
{
    return advanceSearchBar ? advanceSearchBar->isVisible() : false;
}

void DFileManagerWindowPrivate::setAdvanceSearchBarVisible(bool visible)
{
    if (!advanceSearchBar) {
        if (!visible) return;
        initAdvanceSearchBar();
    }

    advanceSearchBar->setVisible(visible);
}

void DFileManagerWindowPrivate::initRenameBar()
{
    if (renameBar) return;

    Q_Q(DFileManagerWindow);

    // see the comment in initAdvanceSearchBar()
    renameBar = new DRenameBar(q);

    rightViewLayout->insertWidget(rightViewLayout->indexOf(emptyTrashButton) + 1, renameBar);

    QObject::connect(renameBar, &DRenameBar::clickCancelButton, q, &DFileManagerWindow::hideRenameBar);
}

bool DFileManagerWindowPrivate::isRenameBarVisible() const
{
    return advanceSearchBar ? advanceSearchBar->isVisible() : false;
}

void DFileManagerWindowPrivate::setRenameBarVisible(bool visible)
{
    if (!renameBar) {
        if (!visible) return;
        initRenameBar();
    }

    renameBar->setVisible(visible);
}

void DFileManagerWindowPrivate::resetRenameBar()
{
    if (!renameBar) return;

    renameBar->resetRenameBar();
}

void DFileManagerWindowPrivate::storeUrlListToRenameBar(const QList<DUrl> &list) noexcept
{
    if (!renameBar) initRenameBar();

    renameBar->storeUrlList(list);
}

DFileManagerWindow::DFileManagerWindow(QWidget *parent)
    : DFileManagerWindow(DUrl(), parent)
{
}

DFileManagerWindow::DFileManagerWindow(const DUrl &fileUrl, QWidget *parent)
    : DMainWindow(parent)
    , d_ptr(new DFileManagerWindowPrivate(this))
{
    if (!DFMGlobal::IsFileManagerDiloagProcess) {
        const QString &currentTheme = DFMApplication::appAttribute(DFMApplication::AA_ThemeName).toString();
        DThemeManager::instance()->setTheme(this, currentTheme);
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

    Tab *tab = d->tabBar->tabAt(index);

    if (!tab) {
        return;
    }

    DFMBaseView *view = tab->fileView();

    d->viewStackLayout->removeWidget(view->widget());
    view->deleteLater();

    d->toolbar->removeNavStackAt(index);
    d->tabBar->removeTab(index, remainState);
}

void DFileManagerWindow::closeCurrentTab(quint64 winId)
{
    D_D(DFileManagerWindow);

    if (winId != this->winId()) {
        return;
    }

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
    DUrl url = DFMApplication::instance()->appUrlAttribute(DFMApplication::AA_UrlOfNewTab);

    if (!url.isValid()) {
        url = currentUrl();
    }

    openNewTab(url);
}

void DFileManagerWindow::requestEmptyTrashFiles()
{
    DFMGlobal::clearTrash();
}

void DFileManagerWindow::onTrashStateChanged()
{
    Q_D(DFileManagerWindow);
    if (currentUrl() == DUrl::fromTrashFile("/") && !TrashManager::isEmpty()) {
        showEmptyTrashButton();
    } else {
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

    Tab *tab = d->tabBar->tabAt(tabIndex);

    if (tab) {
        d->toolbar->switchHistoryStack(tabIndex);

        if (!tab->fileView()) {
            return;
        }

        switchToView(tab->fileView());

//        if (currentUrl().isSearchFile()) {
//            if (!d->toolbar->getSearchBar()->isVisible()) {
//                d->toolbar->searchBarActivated();
//                d->toolbar->getSearchBar()->setText(tab->fileView()->rootUrl().searchKeyword());
//            }
//        } else {
//            if (d->toolbar->getSearchBar()->isVisible()) {
//                d->toolbar->searchBarDeactivated();
//            }
//        }
    }
}

DUrl DFileManagerWindow::currentUrl() const
{
    D_DC(DFileManagerWindow);

    return d->currentView ? d->currentView->rootUrl() : DUrl();
}

DFMBaseView::ViewState DFileManagerWindow::currentViewState() const
{
    D_DC(DFileManagerWindow);

    return d->currentView ? d->currentView->viewState() : DFMBaseView::ViewIdle;
}

bool DFileManagerWindow::isCurrentUrlSupportSearch(const DUrl &currentUrl)
{
    const DAbstractFileInfoPointer &currentFileInfo = DFileService::instance()->createFileInfo(this, currentUrl);

    if (!currentFileInfo || !currentFileInfo->canIteratorDir()) {
        return false;
    }
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

DFMSideBar *DFileManagerWindow::getLeftSideBar() const
{
    D_DC(DFileManagerWindow);

    return d->leftSideBar;
}

int DFileManagerWindow::getSplitterPosition() const
{
    D_DC(DFileManagerWindow);

    return d->splitter ? d->splitter->sizes().at(0) : DFMSideBarItem::maximumWidth;
}

void DFileManagerWindow::setSplitterPosition(int pos)
{
    Q_D(DFileManagerWindow);

    if (d->splitter) {
        d->splitter->setSizes({pos, d->splitter->width() - pos - d->splitter->handleWidth()});
    }
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

bool DFileManagerWindow::cd(const DUrl &fileUrl)
{
    D_D(DFileManagerWindow);

    if (!d->tabBar->currentTab()) {
        d->toolbar->addHistoryStack();
        d->tabBar->createTab(nullptr);
    }

    if (!d->cdForTab(d->tabBar->currentTab(), fileUrl)) {
        return false;
    }

    this->hideRenameBar();

    return true;
}

bool DFileManagerWindow::cdForTab(int tabIndex, const DUrl &fileUrl)
{
    Q_D(DFileManagerWindow);

    return d->cdForTab(d->tabBar->tabAt(tabIndex), fileUrl);
}

bool DFileManagerWindow::cdForTabByView(DFMBaseView *view, const DUrl &fileUrl)
{
    Q_D(DFileManagerWindow);

    for (int i = 0; i < d->tabBar->count(); ++i) {
        Tab *tab =d->tabBar->tabAt(i);

        if (tab->fileView() == view) {
            return d->cdForTab(tab, fileUrl);
        }
    }

    return false;
}

bool DFileManagerWindow::openNewTab(DUrl fileUrl)
{
    D_D(DFileManagerWindow);

    if (!d->tabBar->tabAddable()) {
        return false;
    }

    if (fileUrl.isEmpty()) {
        fileUrl = DUrl::fromLocalFile(QDir::homePath());
    }

    d->toolbar->addHistoryStack();
    d->setCurrentView(0);
    d->tabBar->createTab(Q_NULLPTR);

    return cd(fileUrl);
}

void DFileManagerWindow::switchToView(DFMBaseView *view)
{
    D_D(DFileManagerWindow);

    if (d->currentView == view) {
        return;
    }

    const DUrl &old_url = currentUrl();

    DFMBaseView::ViewState old_view_state = currentViewState();

    d->setCurrentView(view);
    d->viewStackLayout->setCurrentWidget(view->widget());

    if (old_view_state != view->viewState())
        emit currentViewStateChanged();

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
}

void DFileManagerWindow::hideEvent(QHideEvent *event)
{
    QVariantMap state;
    state["sidebar"] = getSplitterPosition();
    DFMApplication::appObtuselySetting()->setValue("WindowManager", "SplitterState", state);

    return DMainWindow::hideEvent(event);
}

void DFileManagerWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    D_DC(DFileManagerWindow);

    if (event->y() <= d->titleFrame->height()) {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
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

    if (!d->processKeyPressEvent(event)) {
        return DMainWindow::keyPressEvent(event);
    }
}

bool DFileManagerWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (!getFileView() || watched != getFileView()->widget()) {
        return false;
    }

    if (event->type() != QEvent::KeyPress) {
        return false;
    }

    Q_D(DFileManagerWindow);

    return d->processKeyPressEvent(static_cast<QKeyEvent *>(event));
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
        if (event->windowId() != this->internalWinId()) {
            return false;
        }

        openNewTab(event.staticCast<DFMUrlBaseEvent>()->url());

        return true;
    default: break;
    }

    return false;
}

QObject *DFileManagerWindow::object() const
{
    return const_cast<DFileManagerWindow *>(this);
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
    QHBoxLayout *titleLayout = new QHBoxLayout;
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

    DFileMenu *menu = fileMenuManger->createToolBarSettingsMenu();

    menu->setProperty("DFileManagerWindow", (quintptr)this);
    menu->setProperty("ToolBarSettingsMenu", true);
    menu->setEventData(DUrl(), DUrlList() << DUrl(), winId(), this);

    QAction *set_theme_action = menu->actionAt(1);
    if (set_theme_action) {
        set_theme_action->setCheckable(true);
        set_theme_action->setText(tr("Dark theme"));
        if (DThemeManager::instance()->theme(this) == "dark") {
            set_theme_action->setChecked(true);
        }
        connect(set_theme_action, &QAction::triggered, this, &DFileManagerWindow::onThemeChanged);
    }

    bool isDXcbPlatform = false;
    SingleApplication *app = static_cast<SingleApplication *>(qApp);
    if (app) {
        isDXcbPlatform = app->isDXcbPlatform();
    }

    if (isDXcbPlatform) {
        d->toolbar->getSettingsButton()->hide();
        titlebar()->setMenu(menu);
        titlebar()->setContentsMargins(0, 0, 0, 0);

        titlebar()->setCustomWidget(d->titleFrame, Qt::AlignLeft);
    } else {
        d->toolbar->getSettingsButton()->setMenu(menu);
    }
}

void DFileManagerWindow::initSplitter()
{
    D_D(DFileManagerWindow);

    initLeftSideBar();
    initRightView();

    d->splitter = new DFMSplitter(Qt::Horizontal, this);
    d->splitter->addWidget(d->leftSideBar);
    d->splitter->addWidget(d->rightView);
    d->splitter->setChildrenCollapsible(false);
}

void DFileManagerWindow::initLeftSideBar()
{
    D_D(DFileManagerWindow);

//    d->leftSideBar = new DLeftSideBar(this);
    d->leftSideBar = new DFMSideBar(this);
    d->leftSideBar->setObjectName("LeftSideBar");
//    d->leftSideBar->setFixedWidth(LEFTSIDEBAR_MAX_WIDTH);
    d->leftSideBar->setMaximumWidth(DFMSideBarItem::maximumWidth);
    d->leftSideBar->setMinimumWidth(DFMSideBarItem::minimumWidth);
    // connections
    connect(this, &DFileManagerWindow::currentUrlChanged, this, [this, d]() {
        d->leftSideBar->setCurrentUrl(currentUrl());
    });
}

void DFileManagerWindow::initRightView()
{
    D_D(DFileManagerWindow);

    initTabBar();
    initViewLayout();
    d->rightView = new QFrame;

    QSizePolicy sp = d->rightView->sizePolicy();

    //NOTE(zccrs): 保证窗口宽度改变时只会调整right view的宽度，侧边栏保持不变
    //             QSplitter是使用QLayout的策略对widgets进行布局，所以此处
    //             设置size policy可以生效
    sp.setHorizontalStretch(1);
    d->rightView->setSizePolicy(sp);

    this->initRenameBarState();

    d->emptyTrashButton = new QPushButton{ this };
    d->emptyTrashButton->setFixedHeight(25);
    d->emptyTrashButton->hide();
    d->emptyTrashButton->setContentsMargins(0, 0, 0, 0);
    d->emptyTrashButton->setObjectName("EmptyTrashButton");

    QHBoxLayout *tabBarLayout = new QHBoxLayout;
    tabBarLayout->setMargin(0);
    tabBarLayout->setSpacing(0);
    tabBarLayout->addWidget(d->tabBar);
    tabBarLayout->addWidget(d->newTabButton);

    d->rightViewLayout = new QVBoxLayout;
    d->rightViewLayout->addLayout(tabBarLayout);
    d->rightViewLayout->addWidget(d->emptyTrashButton);
    d->rightViewLayout->addLayout(d->viewStackLayout);
    d->rightViewLayout->setSpacing(0);
    d->rightViewLayout->setContentsMargins(0, 0, 0, 0);
    d->rightView->setLayout(d->rightViewLayout);
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
    d->newTabButton->setFixedSize(25, 24);
    d->newTabButton->hide();
}

void DFileManagerWindow::initViewLayout()
{
    D_D(DFileManagerWindow);

    d->viewStackLayout = new QStackedLayout;
    d->viewStackLayout->setSpacing(0);
    d->viewStackLayout->setContentsMargins(0, 0, 0, 0);
}

void DFileManagerWindow::initCentralWidget()
{
    D_D(DFileManagerWindow);
    initSplitter();

    d->centralWidget = new QFrame(this);
    d->centralWidget->setObjectName("CentralWidget");
    QVBoxLayout *mainLayout = new QVBoxLayout;
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

    QObject::connect(fileSignalManager, &FileSignalManager::requestCloseCurrentTab, this, &DFileManagerWindow::closeCurrentTab);

    QObject::connect(d->tabBar, &TabBar::tabMoved, d->toolbar, &DToolBar::moveNavStacks);
    QObject::connect(d->tabBar, &TabBar::currentChanged, this, &DFileManagerWindow::onCurrentTabChanged);
    QObject::connect(d->tabBar, &TabBar::tabCloseRequested, this, &DFileManagerWindow::onRequestCloseTab);
    QObject::connect(d->tabBar, &TabBar::tabAddableChanged, this, &DFileManagerWindow::onTabAddableChanged);

    QObject::connect(d->tabBar, &TabBar::tabBarShown, this, &DFileManagerWindow::showNewTabButton);
    QObject::connect(d->tabBar, &TabBar::tabBarHidden, this, &DFileManagerWindow::hideNewTabButton);
    QObject::connect(d->newTabButton, &QPushButton::clicked, this, &DFileManagerWindow::onNewTabButtonClicked);

    QObject::connect(d->emptyTrashButton, &QPushButton::clicked,
                     this, &DFileManagerWindow::requestEmptyTrashFiles, Qt::QueuedConnection);

    QObject::connect(fileSignalManager, &FileSignalManager::trashStateChanged, this, &DFileManagerWindow::onTrashStateChanged);
    QObject::connect(fileSignalManager, &FileSignalManager::currentUrlChanged, this, &DFileManagerWindow::onTrashStateChanged);
    QObject::connect(d->tabBar, &TabBar::currentChanged, this, &DFileManagerWindow::onTrashStateChanged);

    QObject::connect(this, &DFileManagerWindow::currentUrlChanged, this, [this, d] {
        d->tabBar->onCurrentUrlChanged(DFMUrlBaseEvent(this, currentUrl()));
        emit fileSignalManager->currentUrlChanged(DFMUrlBaseEvent(this, currentUrl()));

        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, currentUrl());

        if (info)
        {
            setWindowTitle(info->fileDisplayName());
        } else if (currentUrl().isComputerFile())
        {
            setWindowTitle(systemPathManager->getSystemPathDisplayName("Computer"));
        }
    });

    QObject::connect(fileSignalManager, &FileSignalManager::requestMultiFilesRename, this, &DFileManagerWindow::onShowRenameBar);
    QObject::connect(d->tabBar, &TabBar::currentChanged, this, &DFileManagerWindow::onTabBarCurrentIndexChange);
}

void DFileManagerWindow::moveCenterByRect(QRect rect)
{
    QRect qr = frameGeometry();
    qr.moveCenter(rect.center());
    move(qr.topLeft());
}


void DFileManagerWindow::onShowRenameBar(const DFMUrlListBaseEvent &event) noexcept
{
    DFileManagerWindowPrivate *const d { d_func() };

    if (event.windowId() == this->windowId()) {
        d->storeUrlListToRenameBar(event.urlList()); //### get the urls of selection.

        m_currentTab = d->tabBar->currentTab();
        d->setRenameBarVisible(true);
    }
}

void DFileManagerWindow::onTabBarCurrentIndexChange(const int &index)noexcept
{
    DFileManagerWindowPrivate *const d{ d_func() };

    if (m_currentTab != d->tabBar->tabAt(index)) {

        if (d->isRenameBarVisible() == true) {
            this->onReuqestCacheRenameBarState();//###: invoke this function before setVisible.

            hideRenameBar();
        }
    }
}

void DFileManagerWindow::hideRenameBar() noexcept //###: Hide renamebar and then clear history.
{
    DFileManagerWindowPrivate *const d{ d_func() };

    d->setRenameBarVisible(false);
    d->resetRenameBar();
}


void DFileManagerWindow::onReuqestCacheRenameBarState()const
{
    const DFileManagerWindowPrivate *const d{ d_func() };
    DFileManagerWindow::renameBarState = d->renameBar->getCurrentState();//###: record current state, when a new window is created from a already has tab.
}

void DFileManagerWindow::setTheme(const QString &theme)
{
    DThemeManager::instance()->setTheme(this, theme);
    DFMApplication::setAppAttribute(DFMApplication::AA_ThemeName, theme);

    DFileMenu *dfmenu = static_cast<DFileMenu *>(titlebar()->menu());
    QAction *theme_action = dfmenu->actionAt(1);

    if (theme_action) {
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
    foreach (const QWidget *w, WindowManager::instance()->getWindows().keys()) {
        const DFileManagerWindow *dfm = static_cast<const DFileManagerWindow *>(w);
        const_cast<DFileManagerWindow *>(dfm)->setTheme(theme);
    }

    WindowManager::instance()->saveWindowState(this);
}

void DFileManagerWindow::showEvent(QShowEvent *event)
{
    DMainWindow::showEvent(event);

    const QVariantMap &state = DFMApplication::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
    int splitterPos = state.value("sidebar", DFMSideBarItem::maximumWidth).toInt();
    setSplitterPosition(splitterPos);
}

void DFileManagerWindow::initRenameBarState()
{
    DFileManagerWindowPrivate *const d{ d_func() };

    bool expected{ true };
    ///###: CAS, when we draged a tab to leave TabBar for creating a new window.
    if (DFileManagerWindow::flagForNewWindowFromTab.compare_exchange_strong(expected, false, std::memory_order_seq_cst)) {

        if (static_cast<bool>(DFileManagerWindow::renameBarState) == true) { //###: when we drag a tab to create a new window, but the RenameBar is showing in last window.
            d->renameBar->loadState(DFileManagerWindow::renameBarState);

        } else { //###: when we drag a tab to create a new window, but the RenameBar is hiding.
            d->setRenameBarVisible(false);
        }

    } else { //###: when open a new window from right click menu.
        d->setRenameBarVisible(false);
    }
}


void DFileManagerWindow::requestToSelectUrls()
{
    DFileManagerWindowPrivate *const d{ d_func() };
    if (static_cast<bool>(DFileManagerWindow::renameBarState) == true) {
        d->renameBar->loadState(DFileManagerWindow::renameBarState);

        QList<DUrl> selectedUrls{ DFileManagerWindow::renameBarState->getSelectedUrl() };
        quint64 winId{ this->windowId() };
        DFMUrlListBaseEvent event{ nullptr,  selectedUrls};
        event.setWindowId(winId);

        QTimer::singleShot(100, [ = ] { emit fileSignalManager->requestSelectFile(event); });

        DFileManagerWindow::renameBarState.reset(nullptr);
    }
}

bool DFileManagerWindow::isAdvanceSearchBarVisible()
{
    Q_D(DFileManagerWindow);

    return d->isAdvanceSearchBarVisible();
}

void DFileManagerWindow::toggleAdvanceSearchBar(bool visible, bool resetForm)
{
    Q_D(DFileManagerWindow);

    if (!d->currentView) return;

    if (d->isAdvanceSearchBarVisible() != visible) {
        d->setAdvanceSearchBarVisible(visible);
    }

    if (d->advanceSearchBar && resetForm) {
        d->advanceSearchBar->resetForm();
    }
}
