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
#include "sidebareventcaller.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "services/filemanager/windows/windowsservice.h"

#include <dfm-framework/framework.h>
#include <QUrl>

DPSIDEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

void SideBarEventCaller::sendItemActived(QWidget *sender, const QUrl &url)
{
    quint64 id = windowId(sender);
    dpf::Event event;
    event.setTopic(Sidebar::EventTopic::kSideBarItem);
    event.setData(Sidebar::EventData::kCdAction);
    event.setProperty(Sidebar::EventProperty::kWindowId, id);
    event.setProperty(Sidebar::EventProperty::kUrl, url);
    dpfInstance.eventProxy().pubEvent(event);
}

quint64 SideBarEventCaller::windowId(QWidget *sender)
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    return windowService->findWindowId(sender);
}
