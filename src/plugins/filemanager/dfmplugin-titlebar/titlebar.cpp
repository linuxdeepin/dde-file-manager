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

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/base/urlroute.h"

#include <dfm-framework/framework.h>

DPTITLEBAR_USE_NAMESPACE

namespace GlobalPrivate {
static DSB_FM_NAMESPACE::WindowsService *windowService { nullptr };
}   // namespace GlobalPrivate

void TitleBar::initialize()
{
    DSB_FM_USE_NAMESPACE
    DFMBASE_USE_NAMESPACE

    auto &ctx = dpfInstance.serviceContext();
    Q_ASSERT_X(ctx.loaded(WindowsService::name()), "SideBar", "WindowService not loaded");
    GlobalPrivate::windowService = ctx.service<WindowsService>(WindowsService::name());
    connect(GlobalPrivate::windowService, &WindowsService::windowOpened, this, &TitleBar::onWindowOpened, Qt::DirectConnection);
    connect(GlobalPrivate::windowService, &WindowsService::windowClosed, this, &TitleBar::onWindowClosed, Qt::DirectConnection);

    // file scheme for crumbar
    CrumbManager::instance()->registerCrumbCreator(SchemeTypes::kFile, []() {
        CrumbInterface *interface { new CrumbInterface };
        interface->registewrSupportedUrlCallback(&TitleBarHelper::crumbSupportedUrl);
        interface->registerSeprateUrlCallback(&TitleBarHelper::crumbSeprateUrl);
        return interface;
    });

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

void TitleBar::onWindowOpened(quint64 windId)
{
    auto window = GlobalPrivate::windowService->findWindowById(windId);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    TitleBarWidget *titleBar = new TitleBarWidget;
    window->installTitleBar(titleBar);
    window->installTitleMenu(TitleBarHelper::createSettingsMenu(windId));
    TitleBarHelper::addTileBar(windId, titleBar);
}

void TitleBar::onWindowClosed(quint64 windId)
{
    TitleBarHelper::removeTitleBar(windId);
}
