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
#include "events/titlebareventreceiver.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_titlebar;

namespace GlobalPrivate {
static TitleBarWidget *kTitleBar { nullptr };
}   // namespace GlobalPrivate

void TitleBar::initialize()
{
    DFMBASE_USE_NAMESPACE

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this, &TitleBar::onWindowCreated, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &TitleBar::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &TitleBar::onWindowClosed, Qt::DirectConnection);

    // event has been sended before the Window showed
    bindEvents();

    UrlRoute::regScheme(Global::Scheme::kSmb, "/", {}, true);
    UrlRoute::regScheme(Global::Scheme::kFtp, "/", {}, true);
    UrlRoute::regScheme(Global::Scheme::kSFtp, "/", {}, true);
}

bool TitleBar::start()
{
    DFMBASE_USE_NAMESPACE
    // file scheme for crumbar
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kFile), QVariantMap {});

    return true;
}

dpf::Plugin::ShutdownFlag TitleBar::stop()
{
    return kSync;
}

void TitleBar::onWindowCreated(quint64 windId)
{
    GlobalPrivate::kTitleBar = new TitleBarWidget;
    TitleBarHelper::addTileBar(windId, GlobalPrivate::kTitleBar);
}

void TitleBar::onWindowOpened(quint64 windId)
{
    DFMBASE_USE_NAMESPACE

    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    window->installTitleBar(GlobalPrivate::kTitleBar);
    window->installTitleMenu(TitleBarHelper::createSettingsMenu(windId));

    Q_ASSERT(GlobalPrivate::kTitleBar->navWidget());
    connect(window, &FileManagerWindow::reqBack, GlobalPrivate::kTitleBar->navWidget(), &NavWidget::back);
    connect(window, &FileManagerWindow::reqForward, GlobalPrivate::kTitleBar->navWidget(), &NavWidget::forward);
    // First window's tab created before first url changed in titlebar
    connect(window, &FileManagerWindow::workspaceInstallFinished, GlobalPrivate::kTitleBar->navWidget(),
            &NavWidget::onNewWindowOpended);
    connect(window, &FileManagerWindow::reqSearchCtrlF, GlobalPrivate::kTitleBar, &TitleBarWidget::handleHotkeyCtrlF);
    connect(window, &FileManagerWindow::reqSearchCtrlL, GlobalPrivate::kTitleBar, &TitleBarWidget::handleHotkeyCtrlL);
    connect(window, &FileManagerWindow::reqTriggerActionByIndex, GlobalPrivate::kTitleBar, &TitleBarWidget::handleHotketSwitchViewMode);
}

void TitleBar::onWindowClosed(quint64 windId)
{
    TitleBarHelper::removeTitleBar(windId);
}

void TitleBar::bindEvents()
{
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_Tab_Added",
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabAdded);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_Tab_Changed",
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_Tab_Moved",
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabMoved);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_Tab_Removed",
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabRemovd);

    dpfSignalDispatcher->subscribe(DFMBASE_NAMESPACE::kSwitchViewMode,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleViewModeChanged);

    // bind self slot events  slot_Spinner_Start
    static constexpr auto curSpace { DPF_MACRO_TO_STR(DPTITLEBAR_NAMESPACE) };
    dpfSlotChannel->connect(curSpace, "slot_Custom_Register",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleCustomRegister);
    dpfSlotChannel->connect(curSpace, "slot_Spinner_Start",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleStartSpinner);
    dpfSlotChannel->connect(curSpace, "slot_Spinner_Stop",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleStopSpinner);
    dpfSlotChannel->connect(curSpace, "slot_FilterButton_Show",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleShowFilterButton);
}
