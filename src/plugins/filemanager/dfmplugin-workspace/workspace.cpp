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

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void Workspace::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowsService *windowService = ctx.service<WindowsService>(WindowsService::name());
    Q_ASSERT_X(!windowService->windowIdList().isEmpty(), "Workspace", "Cannot acquire any window");
    // get first window
    quint64 id = windowService->windowIdList().first();
    auto window = windowService->findWindowById(id);
    Q_ASSERT_X(window, "Workspace", "Cannot find window by id");
    window->installWorkSpace(new WorkspaceWidget);
}

bool Workspace::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag Workspace::stop()
{
    return kSync;
}
