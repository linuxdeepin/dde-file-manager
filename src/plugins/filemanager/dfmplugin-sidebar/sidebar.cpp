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
#include "sidebar.h"
#include "views/sidebarwidget.h"
#include "views/sidebaritem.h"
#include "utils/sidebarhelper.h"
#include "events/sidebareventreceiver.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/standardpaths.h"

#include <dfm-framework/framework.h>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void SideBar::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &SideBar::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &SideBar::onWindowClosed, Qt::DirectConnection);
    SideBarEventReceiver::instance()->bindEvents();
}

bool SideBar::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag SideBar::stop()
{
    return kSync;
}

void SideBar::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    auto sidebar = new SideBarWidget;
    SideBarHelper::addSideBar(windId, sidebar);
    window->installSideBar(sidebar);
}

void SideBar::onWindowClosed(quint64 winId)
{
    SideBarHelper::removeSideBar(winId);
}
