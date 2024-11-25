// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebareventreceiver.h"
#include "views/titlebarwidget.h"
#include "views/navwidget.h"
#include "views/tabbar.h"
#include "views/tab.h"
#include "utils/crumbmanager.h"
#include "utils/crumbinterface.h"
#include "utils/titlebarhelper.h"
#include "utils/optionbuttonmanager.h"

#include <dfm-base/utils/universalutils.h>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

TitleBarEventReceiver *TitleBarEventReceiver::instance()
{
    static TitleBarEventReceiver receiver;
    return &receiver;
}

bool TitleBarEventReceiver::handleCustomRegister(const QString &scheme, const QVariantMap &properties)
{
    Q_ASSERT(!scheme.isEmpty());
    if (CrumbManager::instance()->isRegisted(scheme)) {
        fmWarning() << "Crumb sechme " << scheme << "has been resigtered!";
        return false;
    }

    bool keepAddressBar { properties.value(CustomKey::kKeepAddressBar).toBool() };
    bool hideListViewBtn { properties.value(CustomKey::kHideListViewBtn).toBool() };
    bool hideIconViewBtn { properties.value(CustomKey::kHideIconViewBtn).toBool() };
    bool hideTreeViewBtn { properties.value(CustomKey::kHideTreeViewBtn).toBool() };
    bool hideDetailSpaceBtn { properties.value(CustomKey::kHideDetailSpaceBtn).toBool() };

    int state { OptionButtonManager::kDoNotHide };
    if (hideListViewBtn)
        state |= OptionButtonManager::kHideListViewBtn;
    if (hideIconViewBtn)
        state |= OptionButtonManager::kHideIconViewBtn;
    if (hideTreeViewBtn)
        state |= OptionButtonManager::kHideTreeViewBtn;
    if (hideDetailSpaceBtn)
        state |= OptionButtonManager::kHideDetailSpaceBtn;
    if (state != OptionButtonManager::kDoNotHide)
        OptionButtonManager::instance()->setOptBtnVisibleState(scheme, static_cast<OptionButtonManager::OptBtnVisibleState>(state));

    CrumbManager::instance()->registerCrumbCreator(scheme, [=]() {
        CrumbInterface *interface = new CrumbInterface();
        interface->setSupportedScheme(scheme);
        return interface;
    });

    if (keepAddressBar)
        TitleBarHelper::registerKeepTitleStatusScheme(scheme);

    return true;
}

void TitleBarEventReceiver::handleStartSpinner(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->startSpinner();
}

void TitleBarEventReceiver::handleStopSpinner(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->stopSpinner();
}

void TitleBarEventReceiver::handleShowFilterButton(quint64 windowId, bool visible)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->showSearchFilterButton(visible);
}

void TitleBarEventReceiver::handleViewModeChanged(quint64 windowId, int mode)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;

    w->setViewModeState(mode);
}

void TitleBarEventReceiver::handleSetNewWindowAndTabEnable(bool enable)
{
    TitleBarHelper::newWindowAndTabEnabled = enable;
}

void TitleBarEventReceiver::handleWindowForward(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->navWidget()->forward();
}

void TitleBarEventReceiver::handleWindowBackward(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->navWidget()->back();
}

void TitleBarEventReceiver::handleRemoveHistory(quint64 windowId, const QUrl &url)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->navWidget()->removeUrlFromHistoryStack(url);
}

TitleBarEventReceiver::TitleBarEventReceiver(QObject *parent)
    : QObject(parent)
{
}

bool TitleBarEventReceiver::handleTabAddable(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return false;

    return w->tabBar()->tabAddable();
}

void TitleBarEventReceiver::handleCloseTabs(const QUrl &url)
{
    auto titlebarWidges = TitleBarHelper::titlebars();

    for (auto w : titlebarWidges)
        w->tabBar()->closeTab(url);
}

void TitleBarEventReceiver::handleSetTabAlias(const QUrl &url, const QString &name)
{
    auto titlebarWidges = TitleBarHelper::titlebars();

    for (auto w : titlebarWidges) {
        auto tabBar = w->tabBar();
        for (int i = 0; i < tabBar->count(); ++i) {
            auto tab = tabBar->tabAt(i);
            if (tab && UniversalUtils::urlEquals(url, tab->getCurrentUrl()))
                tab->setTabAlias(name);
        }
    }
}

void TitleBarEventReceiver::handleOpenNewTabTriggered(quint64 windowId, const QUrl &url)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;

    w->openNewTab(url);
}
