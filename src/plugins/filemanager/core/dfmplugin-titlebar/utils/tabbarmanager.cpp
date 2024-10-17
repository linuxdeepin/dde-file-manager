// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbarmanager.h"
#include "views/tabbar.h"
#include "views/tab.h"
#include "views/titlebarwidget.h"
#include "events/titlebareventcaller.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-framework/dpf.h>

DFMBASE_USE_NAMESPACE
DPTITLEBAR_USE_NAMESPACE

TabBarManager *TabBarManager::instance()
{
    static TabBarManager instance;
    return &instance;
}

TabBarManager::TabBarManager(QObject *parent)
    : QObject(parent)
{
}

TabBarManager::~TabBarManager()
{
}

TabBar *TabBarManager::createTabBar(quint64 windowId, QWidget *parent)
{
    if (!parent)
        return nullptr;

    if (tabBars.contains(windowId)) {
        return tabBars[windowId];
    }

    TabBar *tabBar = new TabBar(parent);
    tabBars[windowId] = tabBar;
    tabBarsByWidget[parent] = tabBar;
    connect(tabBar, &TabBar::currentChanged, this, [=](int tabIndex) {
        Tab *tab = tabBar->tabAt(tabIndex);
        if (tab) {
            // switch tab must before change url! otherwise NavWidget can not work!
            auto titleWidget = TitleBarHelper::findTileBarByWindowId(windowId);
            titleWidget->currentTabChanged(tabIndex);
            TitleBarEventCaller::sendChangeCurrentUrl(windowId, tab->getCurrentUrl());
        }
    });
    connect(tabBar, &TabBar::tabCloseRequested, this, [=](int index, bool remainState) {
        tabBar->removeTab(index, remainState);
    });
    connect(tabBar, &TabBar::tabAddButtonClicked, this, [=]() {
        QUrl url = Application::instance()->appUrlAttribute(Application::kUrlOfNewTab);
        if (!url.isValid()) {
            url = tabBar->currentTab()->getCurrentUrl();
        }

        openNewTab(windowId, url);
    });
    return tabBar;
}

void TabBarManager::removeTabBar(const quint64 windowId)
{
    if (tabBars.contains(windowId)) {
        delete tabBars[windowId];
        tabBars.remove(windowId);
    }
}

void TabBarManager::setCurrentUrl(QWidget *parent, const QUrl &url)
{
    auto tabBar = tabBarsByWidget[parent];
    if (!tabBar)
        return;

    tabBar->setCurrentUrl(url);
}

void TabBarManager::openNewTab(const quint64 windowId, const QUrl &url)
{
    if (!tabBars.contains(windowId))
        return;

    auto tabBar = tabBars[windowId];
    if (!tabBar->tabAddable())
        return;

    tabBar->createTab();

    if (url.isEmpty())
        TitleBarEventCaller::sendCd(tabBar, StandardPaths::location(StandardPaths::kHomePath));

    TitleBarEventCaller::sendCd(tabBar, url);
}

bool TabBarManager::canAddNewTab(const quint64 windowId)
{
    if (!tabBars.contains(windowId))
        return false;

    return tabBars[windowId]->tabAddable();
}

void TabBarManager::closeTab(const QUrl &url)
{
    for (auto tabBar : tabBars) {
        tabBar->closeTab(url);
    }
}

void TabBarManager::setTabAlias(const QUrl &url, const QString &newName)
{
    for (auto tabBar : tabBars) {
        for (int i = 0; i < tabBar->count(); ++i) {
            auto tab = tabBar->tabAt(i);
            if (tab && UniversalUtils::urlEquals(url, tab->getCurrentUrl()))
                tab->setTabAlias(newName);
        }
    }
}

void TabBarManager::closeCurrentTab(const quint64 windowId)
{
    if (!tabBars.contains(windowId))
        return;

    auto tabBar = tabBars[windowId];
    if (tabBar->count() == 1) {
        auto window = FMWindowsIns.findWindowById(windowId);
        if (window)
            window->close();

        return;
    }

    tabBar->removeTab(tabBar->getCurrentIndex());
}

void TabBarManager::activateNextTab(const quint64 windowId)
{
    if (!tabBars.contains(windowId))
        return;

    auto tabBar = tabBars[windowId];
    tabBar->activateNextTab();
}

void TabBarManager::activatePreviousTab(const quint64 windowId)
{
    if (!tabBars.contains(windowId))
        return;

    auto tabBar = tabBars[windowId];
    tabBar->activatePreviousTab();
}

void TabBarManager::createNewTab(const quint64 windowId)
{
    // If a directory is selected, open NewTab through the URL of the selected directory
    QList<QUrl> urls = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", windowId).value<QList<QUrl>>();
    if (urls.count() == 1) {
        const FileInfoPointer &fileInfoPtr = InfoFactory::create<FileInfo>(urls.at(0));
        if (fileInfoPtr && fileInfoPtr->isAttributes(OptInfoType::kIsDir)) {
            openNewTab(windowId, urls.at(0));
            return;
        }
    }

    openNewTab(windowId, getCurrentUrl(windowId));
}

void TabBarManager::activateTab(const quint64 windowId, const int index)
{
    if (!tabBars.contains(windowId))
        return;

    auto tabBar = tabBars[windowId];
    tabBar->setCurrentIndex(index);
}

void TabBarManager::checkCurrentTab(const quint64 windowId)
{
    if (!tabBars.contains(windowId))
        return;

    auto tabBar = tabBars[windowId];
    if (!tabBar->currentTab())
        tabBar->createTab();
}

QUrl TabBarManager::getCurrentUrl(const quint64 windowId)
{
    if (!tabBars.contains(windowId))
        return QUrl();

    auto tab = tabBars[windowId]->currentTab();
    if (!tab)
        return QUrl();

    return tab->getCurrentUrl();
}
