// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebareventreceiver.h"
#include "views/titlebarwidget.h"
#include "views/navwidget.h"
#include "views/tabbar.h"
#include "utils/crumbmanager.h"
#include "utils/crumbinterface.h"
#include "utils/titlebarhelper.h"
#include "utils/optionbuttonmanager.h"

#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/event/eventhelper.h>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

TitleBarEventReceiver *TitleBarEventReceiver::instance()
{
    static TitleBarEventReceiver receiver;
    return &receiver;
}

bool TitleBarEventReceiver::handleCustomRegister(const QString &scheme, const QVariantMap &properties)
{
    Q_ASSERT(!scheme.isEmpty());
    if (CrumbManager::instance()->isRegistered(scheme)) {
        fmWarning() << "Crumb scheme" << scheme << "has already been registered";
        return false;
    }

    bool keepAddressBar { properties.value(CustomKey::kKeepAddressBar).toBool() };
    bool hideDetailSpaceBtn { properties.value(CustomKey::kHideDetailSpaceBtn).toBool() };
    bool hideListViewBtn = properties.contains(ViewCustomKeys::kSupportListMode)
            && !properties.value(ViewCustomKeys::kSupportListMode).toBool();
    bool hideIconViewBtn = properties.contains(ViewCustomKeys::kSupportIconMode)
            && !properties.value(ViewCustomKeys::kSupportIconMode).toBool();
    bool hideTreeViewBtn = properties.contains(ViewCustomKeys::kSupportTreeMode)
            && !properties.value(ViewCustomKeys::kSupportTreeMode).toBool();
    bool hideListHeightOpt = properties.contains(ViewCustomKeys::kAllowChangeListHeight)
            && !properties.value(ViewCustomKeys::kAllowChangeListHeight).toBool();
    ViewModeUrlCallback modelViewUrlCallback = DPF_NAMESPACE::paramGenerator<ViewModeUrlCallback>(properties.value(ViewCustomKeys::kViewModeUrlCallback));

    int state { OptionButtonManager::kDoNotHide };
    if (hideListViewBtn)
        state |= OptionButtonManager::kHideListViewBtn;
    if (hideIconViewBtn)
        state |= OptionButtonManager::kHideIconViewBtn;
    if (hideTreeViewBtn)
        state |= OptionButtonManager::kHideTreeViewBtn;
    if (hideDetailSpaceBtn)
        state |= OptionButtonManager::kHideDetailSpaceBtn;
    if (hideListHeightOpt)
        state |= OptionButtonManager::kHideListHeightOpt;
    if (state != OptionButtonManager::kDoNotHide)
        OptionButtonManager::instance()->setOptBtnVisibleState(scheme, static_cast<OptionButtonManager::OptBtnVisibleState>(state));

    CrumbManager::instance()->registerCrumbCreator(scheme, [=]() {
        CrumbInterface *interface = new CrumbInterface();
        interface->setSupportedScheme(scheme);
        return interface;
    });

    if (keepAddressBar)
        TitleBarHelper::registerKeepTitleStatusScheme(scheme);

    if (modelViewUrlCallback)
        TitleBarHelper::registerViewModelUrlCallback(scheme, modelViewUrlCallback);

    return true;
}

void TitleBarEventReceiver::handleStartSpinner(quint64 windowId)
{
}

void TitleBarEventReceiver::handleStopSpinner(quint64 windowId)
{
}

void TitleBarEventReceiver::handleShowFilterButton(quint64 windowId, bool visible)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w) {
        fmWarning() << "Cannot show filter button: titlebar widget not found for window id" << windowId;
        return;
    }
    w->showSearchFilterButton(visible);
}

void TitleBarEventReceiver::handleViewModeChanged(quint64 windowId, int mode)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w) {
        fmWarning() << "Cannot change view mode: titlebar widget not found for window id" << windowId;
        return;
    }

    w->setViewModeState(mode);
}

void TitleBarEventReceiver::handleSetNewWindowAndTabEnable(bool enable)
{
    TitleBarHelper::newWindowAndTabEnabled = enable;
}

void TitleBarEventReceiver::handleWindowForward(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w) {
        fmWarning() << "Cannot navigate forward: titlebar widget not found for window id" << windowId;
        return;
    }
    w->navWidget()->forward();
}

void TitleBarEventReceiver::handleWindowBackward(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w) {
        fmWarning() << "Cannot navigate backward: titlebar widget not found for window id" << windowId;
        return;
    }
    w->navWidget()->back();
}

void TitleBarEventReceiver::handleRemoveHistory(quint64 windowId, const QUrl &url)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w) {
        fmWarning() << "Cannot remove URL from history: titlebar widget not found for window id" << windowId;
        return;
    }
    w->navWidget()->removeUrlFromHistoryStack(url);
}

TitleBarEventReceiver::TitleBarEventReceiver(QObject *parent)
    : QObject(parent)
{
}

bool TitleBarEventReceiver::handleTabAddable(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w) {
        fmWarning() << "Cannot check tab addable: titlebar widget not found for window id" << windowId;
        return false;
    }

    return true;
}

void TitleBarEventReceiver::handleCloseTabs(const QUrl &url)
{
    auto titlebarWidges = TitleBarHelper::titlebars();

    fmDebug() << "Closing tabs with URL:" << url.toString();
    for (auto w : titlebarWidges)
        w->tabBar()->closeTab(url);
}

void TitleBarEventReceiver::handleSetTabAlias(const QUrl &url, const QString &name)
{
    auto titlebarWidges = TitleBarHelper::titlebars();

    for (auto w : titlebarWidges) {
        auto tabBar = w->tabBar();
        for (int i = 0; i < tabBar->count(); ++i) {
            if (UniversalUtils::urlEquals(url, tabBar->tabUrl(i))) {
                tabBar->setTabAlias(i, name);
            }
        }
    }
}

void TitleBarEventReceiver::handleOpenNewTabTriggered(quint64 windowId, const QUrl &url)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w) {
        fmWarning() << "Cannot open new tab: titlebar widget not found for window id" << windowId;
        return;
    }

    fmDebug() << "Opening new tab for window id:" << windowId << "URL:" << url.toString();
    w->openNewTab(url);
}

void TitleBarEventReceiver::handleUpdateCrumb(const QUrl &url)
{
    auto titlebarWidges = TitleBarHelper::titlebars();
    for (auto w : titlebarWidges) {
        auto crumbBar = w->titleCrumbBar();
        if (crumbBar)
            crumbBar->onUrlChanged(crumbBar->lastUrl());
    }
}
