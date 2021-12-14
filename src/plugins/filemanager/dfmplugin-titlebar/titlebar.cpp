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
#include "titlebarwidget.h"

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/widgets/filemanagerwindow/filemanagerwindow.h"

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE

void TitleBar::initialize()
{
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();

    WindowsService *windowService = ctx.service<WindowsService>(WindowsService::name());
    Q_ASSERT_X(!windowService->windowIdList().isEmpty(), "TitleBar", "Cannot acquire any window");

    // get first window
    quint64 id = windowService->windowIdList().first();
    auto window = windowService->findWindowById(id);
    qDebug() << window->rootUrl();
    Q_ASSERT_X(window, "TitleBar", "Cannot find window by id");
    window->setTitleBar(new TitleBarWidget);
}

bool TitleBar::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag TitleBar::stop()
{
    return kSync;
}
