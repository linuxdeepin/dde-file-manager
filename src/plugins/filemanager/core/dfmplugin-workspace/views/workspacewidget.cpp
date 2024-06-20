// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"
#include "fileview.h"
#include "tabbar.h"
#include "tab.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"
#include "utils/customtopwidgetinterface.h"

#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/event/event.h>

#include <DIconButton>
#include <DHorizontalLine>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QVBoxLayout>
#include <QStackedLayout>
#include <QKeyEvent>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

WorkspaceWidget::WorkspaceWidget(QFrame *parent)
    : AbstractFrame(parent)
{
    initializeUi();
    initConnect();
}

WorkspaceWidget::ViewPtr WorkspaceWidget::currentViewPtr() const
{
    auto scheme = currentUrl().scheme();
    return views.value(scheme);
}

Global::ViewMode WorkspaceWidget::currentViewMode() const
{
    auto scheme = currentUrl().scheme();
    auto view = views.value(scheme);
    if (!view)
        return Global::ViewMode::kNoneMode;

    auto fileView = dynamic_cast<FileView *>(view.data());
    if (fileView)
        return fileView->currentViewMode();

    return Global::ViewMode::kNoneMode;
}

void WorkspaceWidget::setCurrentUrl(const QUrl &url)
{
    auto curView = currentViewPtr();
    if (curView) {
        FileView *view = qobject_cast<FileView *>(curView->widget());
        if (view)
            view->stopWork();
    }

    workspaceUrl = url;

    if (!tabBar->currentTab())
        tabBar->createTab();

    // NOTE: In the function `initCustomTopWidgets` the `cd` event may be
    // called causing this function to reentrant!!!
    if (workspaceUrl != url)
        return;

    QString scheme { url.scheme() };

    if (!views.contains(scheme)) {
        QString error;
        ViewPtr fileView = ViewFactory::create<AbstractBaseView>(url, &error);
        if (!fileView) {
            fmWarning() << "Cannot create view for " << url << "Reason: " << error;
            return;
        }
#ifdef ENABLE_TESTING
        dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                             qobject_cast<QWidget *>(fileView->widget()), AcName::kAcFileView);
#endif

        views.insert(url.scheme(), fileView);
        viewStackLayout->addWidget(fileView->widget());
    }

    setCurrentView(url);
}

QUrl WorkspaceWidget::currentUrl() const
{
    return workspaceUrl;
}

AbstractBaseView *WorkspaceWidget::currentView()
{
    auto scheme = currentUrl().scheme();
    return views.value(scheme).data();
}

void WorkspaceWidget::openNewTab(const QUrl &url)
{
    if (!tabBar->tabAddable())
        return;

    tabBar->createTab();

    auto windowID = WorkspaceHelper::instance()->windowId(this);
    if (url.isEmpty())
        WorkspaceEventCaller::sendChangeCurrentUrl(windowID, StandardPaths::location(StandardPaths::kHomePath));

    WorkspaceEventCaller::sendChangeCurrentUrl(windowID, url);
}

bool WorkspaceWidget::canAddNewTab()
{
    if (tabBar)
        return tabBar->tabAddable();

    return false;
}

void WorkspaceWidget::closeTab(quint64 winId, const QUrl &url)
{
    if (tabBar)
        tabBar->closeTab(winId, url);
}

void WorkspaceWidget::setTabAlias(const QUrl &url, const QString &newName)
{
    if (tabBar) {
        for (int i = 0; i < tabBar->count(); ++i) {
            auto tab = tabBar->tabAt(i);
            if (tab && UniversalUtils::urlEquals(url, tab->getCurrentUrl()))
                tab->setTabAlias(newName);
        }
    }
}

void WorkspaceWidget::setCustomTopWidgetVisible(const QString &scheme, bool visible)
{
    if (topWidgets.contains(scheme)) {
        topWidgets[scheme]->setVisible(visible);
    } else {
        auto interface = WorkspaceHelper::instance()->createTopWidgetByScheme(scheme);
        if (interface) {
            TopWidgetPtr topWidgetPtr = QSharedPointer<QWidget>(interface->create(this));
            if (topWidgetPtr) {
                widgetLayout->insertWidget(widgetLayout->indexOf(tabBottomLine) + 1, topWidgetPtr.get());
                topWidgets.insert(scheme, topWidgetPtr);
                topWidgetPtr->setVisible(visible);
            }
        }
    }
}

bool WorkspaceWidget::getCustomTopWidgetVisible(const QString &scheme)
{
    if (topWidgets.contains(scheme)) {
        return topWidgets[scheme]->isVisible();
    }
    return false;
}

QRectF WorkspaceWidget::viewVisibleGeometry()
{
    FileView *view = dynamic_cast<FileView *>(currentView());
    if (view) {
        QRectF localRect = view->geometry();
        QPoint topLeft(static_cast<int>(localRect.x()), static_cast<int>(localRect.y()));

        QRectF globalRect(view->viewport()->mapToGlobal(topLeft), QSizeF(localRect.width(), localRect.height()));

        return globalRect;
    }

    return {};
}

QRectF WorkspaceWidget::itemRect(const QUrl &url, const Global::ItemRoles role)
{
    FileView *view = dynamic_cast<FileView *>(currentView());
    if (view) {
        QRectF localRect = view->itemRect(url, role);
        QPoint topLeft(static_cast<int>(localRect.x()), static_cast<int>(localRect.y()));

        QRectF globalRect(view->viewport()->mapToGlobal(topLeft), QSizeF(localRect.width(), localRect.height()));

        return globalRect;
    }

    return {};
}

void WorkspaceWidget::onCloseCurrentTab()
{
    if (tabBar->count() == 1) {
        auto winId = WorkspaceHelper::instance()->windowId(this);
        auto window = FMWindowsIns.findWindowById(winId);
        if (window)
            window->close();

        return;
    }

    tabBar->removeTab(tabBar->getCurrentIndex());
}

void WorkspaceWidget::onSetCurrentTabIndex(const int index)
{
    tabBar->setCurrentIndex(index);
}

void WorkspaceWidget::onRefreshCurrentView()
{
    currentView()->refresh();
}

void WorkspaceWidget::onOpenUrlInNewTab(quint64 windowId, const QUrl &url)
{
    quint64 thisWindowID = WorkspaceHelper::instance()->windowId(this);
    if (thisWindowID == windowId)
        openNewTab(url);
}

void WorkspaceWidget::onCurrentTabChanged(int tabIndex)
{
    Tab *tab = tabBar->tabAt(tabIndex);
    if (tab) {
        auto windowID = WorkspaceHelper::instance()->windowId(this);
        // switch tab must before change url! otherwise NavWidget can not work!
        WorkspaceEventCaller::sendTabChanged(windowID, tabIndex);
        WorkspaceEventCaller::sendChangeCurrentUrl(windowID, tab->getCurrentUrl());
    }
}

void WorkspaceWidget::onRequestCloseTab(const int index, const bool &remainState)
{
    tabBar->removeTab(index, remainState);
}

void WorkspaceWidget::onTabAddableChanged(bool addable)
{
    newTabButton->setEnabled(addable);
}

void WorkspaceWidget::showNewTabButton()
{
    newTabButton->show();
    tabTopLine->show();
    tabBottomLine->show();
}

void WorkspaceWidget::hideNewTabButton()
{
    newTabButton->hide();
    tabTopLine->hide();
    tabBottomLine->hide();
}

void WorkspaceWidget::onNewTabButtonClicked()
{
    QUrl url = Application::instance()->appUrlAttribute(Application::kUrlOfNewTab);

    if (!url.isValid()) {
        url = currentUrl();
    }

    openNewTab(url);
}

void WorkspaceWidget::onNextTab()
{
    tabBar->activateNextTab();
}

void WorkspaceWidget::onPreviousTab()
{
    tabBar->activatePreviousTab();
}

void WorkspaceWidget::onCreateNewTab()
{
    // If a directory is selected, open NewTab through the URL of the selected directory
    auto view = currentView();
    if (view) {
        const QList<QUrl> &urls = view->selectedUrlList();
        if (urls.count() == 1) {
            const FileInfoPointer &fileInfoPtr = InfoFactory::create<FileInfo>(urls.at(0));
            if (fileInfoPtr && fileInfoPtr->isAttributes(OptInfoType::kIsDir)) {
                openNewTab(urls.at(0));
                return;
            }
        }
    }
    openNewTab(tabBar->currentTab()->getCurrentUrl());
}

void WorkspaceWidget::showEvent(QShowEvent *event)
{
    AbstractFrame::showEvent(event);

    setFocus();
}

void WorkspaceWidget::focusInEvent(QFocusEvent *event)
{
    FileView *view = dynamic_cast<FileView *>(currentView());
    if (view && !view->hasFocus())
        view->setFocus();

    AbstractFrame::focusInEvent(event);
}

// NOTE(zhangs): please ref to: DFileManagerWindow::initRightView (old filemanager)
void WorkspaceWidget::initializeUi()
{
    initTabBar();
    initViewLayout();
}

void WorkspaceWidget::initConnect()
{
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::openNewTab, this, &WorkspaceWidget::onOpenUrlInNewTab);

    QObject::connect(tabBar, &TabBar::currentChanged, this, &WorkspaceWidget::onCurrentTabChanged);
    QObject::connect(tabBar, &TabBar::tabCloseRequested, this, &WorkspaceWidget::onRequestCloseTab);
    QObject::connect(tabBar, &TabBar::tabAddableChanged, this, &WorkspaceWidget::onTabAddableChanged);
    QObject::connect(tabBar, &TabBar::tabBarShown, this, &WorkspaceWidget::showNewTabButton);
    QObject::connect(tabBar, &TabBar::tabBarHidden, this, &WorkspaceWidget::hideNewTabButton);
    QObject::connect(newTabButton, &DIconButton::clicked, this, &WorkspaceWidget::onNewTabButtonClicked);
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        initUiForSizeMode();
    });
#endif
}

void WorkspaceWidget::initTabBar()
{
    tabBar = new TabBar(this);

    newTabButton = new DIconButton(this);
    newTabButton->setObjectName("NewTabButton");
    newTabButton->setIconSize({ 24, 24 });
    newTabButton->setIcon(QIcon::fromTheme("dfm_tab_new"));
    newTabButton->setFlat(true);
    newTabButton->hide();

    initUiForSizeMode();

    tabTopLine = new DHorizontalLine(this);
    tabBottomLine = new DHorizontalLine(this);
    tabTopLine->setFixedHeight(1);
    tabBottomLine->setFixedHeight(1);
    tabTopLine->hide();
    tabBottomLine->hide();

    tabBarLayout = new QHBoxLayout;
    tabBarLayout->setContentsMargins(0, 0, 0, 0);
    tabBarLayout->setSpacing(0);
    tabBarLayout->addWidget(tabBar);
    tabBarLayout->addWidget(newTabButton);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<TabBar *>(tabBar), AcName::kAcViewTabBar);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<DHorizontalLine *>(tabTopLine), AcName::kAcViewTabBarTopLine);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<DIconButton *>(newTabButton), AcName::kAcViewTabBarNewButton);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<DHorizontalLine *>(tabBottomLine), AcName::kAcViewTabBarBottomLine);
#endif
}

void WorkspaceWidget::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    int size = DSizeModeHelper::element(24, 36);
    tabBar->setFixedHeight(size);
    newTabButton->setFixedSize(size, size);
#else
    tabBar->setFixedHeight(36);
    newTabButton->setFixedSize(36, 36);
#endif
}

void WorkspaceWidget::initViewLayout()
{
    viewStackLayout = new QStackedLayout;
    viewStackLayout->setSpacing(0);
    viewStackLayout->setContentsMargins(0, 0, 0, 0);

    widgetLayout = new QVBoxLayout;
    widgetLayout->addWidget(tabTopLine);
    widgetLayout->addLayout(tabBarLayout);
    widgetLayout->addWidget(tabBottomLine);
    widgetLayout->addLayout(viewStackLayout, 1);
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(widgetLayout);
}

void WorkspaceWidget::onCreateNewWindow()
{
    ViewPtr fileView = views[workspaceUrl.scheme()];
    if (!fileView) {
        fmWarning() << "Cannot find view by url: " << workspaceUrl;
        return;
    }

    QList<QUrl> urlList;
    for (const auto &url : fileView->selectedUrlList()) {
        const auto &info = InfoFactory::create<FileInfo>(url);
        if (info && info->canAttributes(CanableInfoType::kCanFetch))
            urlList << url;
    }

    WorkspaceEventCaller::sendOpenWindow(urlList);
}

void WorkspaceWidget::initCustomTopWidgets(const QUrl &url)
{
    QString scheme { url.scheme() };

    for (auto widget : topWidgets.values()) {
        if (topWidgets.value(scheme) != widget)
            widget->hide();
    }

    auto interface = WorkspaceHelper::instance()->createTopWidgetByUrl(url);
    if (interface && !interface->parent())
        interface->setParent(this);

    if (topWidgets.contains(scheme)) {
        bool showUrl { interface->isShowFromUrl(topWidgets[scheme].data(), url) };
        fmDebug() << interface->isKeepShow() << showUrl;
        topWidgets[scheme]->setVisible(interface && (showUrl || interface->isKeepShow()));
        fmDebug() << topWidgets[scheme]->contentsMargins();
    } else {
        if (interface) {
            TopWidgetPtr topWidgetPtr = QSharedPointer<QWidget>(interface->create());
            if (topWidgetPtr) {
                widgetLayout->insertWidget(widgetLayout->indexOf(tabBottomLine) + 1, topWidgetPtr.get());
                topWidgets.insert(scheme, topWidgetPtr);
                topWidgetPtr->setVisible(interface->isShowFromUrl(topWidgets[scheme].data(), url) || interface->isKeepShow());
            }
        }
    }
}

void WorkspaceWidget::setCurrentView(const QUrl &url)
{
    auto view = views[url.scheme()];
    if (view) {
        viewStackLayout->setCurrentWidget(view->widget());

        view->setRootUrl(url);
        tabBar->setCurrentUrl(url);

        initCustomTopWidgets(url);
    }
}
