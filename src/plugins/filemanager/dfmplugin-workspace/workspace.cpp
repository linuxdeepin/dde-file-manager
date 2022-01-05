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
#include "workspace.h"
#include "workspacewidget.h"
#include "fileview.h"
#include "utils/workspacehelper.h"
#include "events/workspaceeventreceiver.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/filemanager/titlebar/titlebar_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

namespace GlobalPrivate {
static WindowsService *windowService { nullptr };
}   // namespace GlobalPrivate

void Workspace::initialize()
{
    ViewFactory::regClass<FileView>(SchemeTypes::kFile);

    auto &ctx = dpfInstance.serviceContext();
    Q_ASSERT_X(ctx.loaded(WindowsService::name()), "Workspace", "WindowService not loaded");
    GlobalPrivate::windowService = ctx.service<WindowsService>(WindowsService::name());
    connect(GlobalPrivate::windowService, &WindowsService::windowOpened, this, &Workspace::onWindowOpened, Qt::DirectConnection);
    connect(GlobalPrivate::windowService, &WindowsService::windowClosed, this, &Workspace::onWindowClosed, Qt::DirectConnection);
}

bool Workspace::start()
{
    dpfInstance.eventDispatcher().subscribe(TitleBar::EventType::kSwitchMode,
                                            WorkspaceEventReceiver::instance(), &WorkspaceEventReceiver::handleTileBarSwitchModeTriggered);
    return true;
}

dpf::Plugin::ShutdownFlag Workspace::stop()
{
    return kSync;
}

void Workspace::onWindowOpened(quint64 windId)
{
    auto window = GlobalPrivate::windowService->findWindowById(windId);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    WorkspaceWidget *workspace = new WorkspaceWidget;
    window->installWorkSpace(workspace);
    WorkspaceHelper::instance()->addWorkspace(windId, workspace);
}

void Workspace::onWindowClosed(quint64 windId)
{
    // TODO(zhangs): impl me!
    WorkspaceHelper::instance()->removeWorkspace(windId);
}
