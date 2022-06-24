/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "titlebar.h"
#include "utils/titlebarhelper.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"
#include "views/titlebarwidget.h"
#include "events/titlebarunicastreceiver.h"
#include "events/titlebareventreceiver.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/workspace/workspace_defines.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

DPTITLEBAR_USE_NAMESPACE

namespace GlobalPrivate {
static TitleBarWidget *titleBar { nullptr };
}   // namespace GlobalPrivate

void TitleBar::initialize()
{
    DSB_FM_USE_NAMESPACE
    DFMBASE_USE_NAMESPACE

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this, &TitleBar::onWindowCreated, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &TitleBar::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &TitleBar::onWindowClosed, Qt::DirectConnection);

    // file scheme for crumbar
    CrumbManager::instance()->registerCrumbCreator(Global::Scheme::kFile, []() {
        CrumbInterface *interface { new CrumbInterface };
        interface->registewrSupportedUrlCallback(&TitleBarHelper::crumbSupportedUrl);
        interface->registerSeprateUrlCallback(&TitleBarHelper::crumbSeprateUrl);
        return interface;
    });

    // event has been sended before the Window showed
    bindEvents();

    UrlRoute::regScheme(Global::Scheme::kSmb, "/", {}, true);
    UrlRoute::regScheme(Global::Scheme::kFtp, "/", {}, true);
    UrlRoute::regScheme(Global::Scheme::kSFtp, "/", {}, true);

    TitleBarUnicastReceiver::instance()->connectService();
}

bool TitleBar::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag TitleBar::stop()
{
    return kSync;
}

void TitleBar::onWindowCreated(quint64 windId)
{
    GlobalPrivate::titleBar = new TitleBarWidget;
    TitleBarHelper::addTileBar(windId, GlobalPrivate::titleBar);
}

void TitleBar::onWindowOpened(quint64 windId)
{
    DFMBASE_USE_NAMESPACE

    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    window->installTitleBar(GlobalPrivate::titleBar);
    window->installTitleMenu(TitleBarHelper::createSettingsMenu(windId));

    Q_ASSERT(GlobalPrivate::titleBar->navWidget());
    connect(window, &FileManagerWindow::reqBack, GlobalPrivate::titleBar->navWidget(), &NavWidget::back);
    connect(window, &FileManagerWindow::reqForward, GlobalPrivate::titleBar->navWidget(), &NavWidget::forward);
    // First window's tab created before first url changed in titlebar
    connect(window, &FileManagerWindow::workspaceInstallFinished, GlobalPrivate::titleBar->navWidget(),
            &NavWidget::onNewWindowOpended);
    connect(window, &FileManagerWindow::reqSearchCtrlF, GlobalPrivate::titleBar, &TitleBarWidget::handleHotkeyCtrlF);
    connect(window, &FileManagerWindow::reqSearchCtrlL, GlobalPrivate::titleBar, &TitleBarWidget::handleHotkeyCtrlL);
    connect(window, &FileManagerWindow::reqTriggerActionByIndex, GlobalPrivate::titleBar, &TitleBarWidget::handleHotketSwitchViewMode);
}

void TitleBar::onWindowClosed(quint64 windId)
{
    TitleBarHelper::removeTitleBar(windId);
}

void TitleBar::bindEvents()
{
    DSB_FM_USE_NAMESPACE
    dpfSignalDispatcher->subscribe(Workspace::EventType::kTabAdded,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabAdded);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kTabChanged,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabChanged);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kTabMoved,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabMoved);
    dpfSignalDispatcher->subscribe(Workspace::EventType::kTabRemoved,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabRemovd);

    dpfSignalDispatcher->subscribe(DFMBASE_NAMESPACE::kSwitchViewMode,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleViewModeChanged);

    // bind self slot events  slot_StartSpinner
    static constexpr auto curSpace { DPF_MACRO_TO_STR(DPTITLEBAR_NAMESPACE) };
    dpfSlotChannel->connect(curSpace, "slot_StartSpinner",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleStartSpinner);
    dpfSlotChannel->connect(curSpace, "slot_StopSpinner",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleStopSpinner);
    dpfSlotChannel->connect(curSpace, "slot_ShowFilterButton",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleShowFilterButton);
}
