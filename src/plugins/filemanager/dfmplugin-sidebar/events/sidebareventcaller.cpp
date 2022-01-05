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
#include "utils/sidebarhelper.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>
#include <QUrl>

DPSIDEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static DPF_NAMESPACE::EventDispatcherManager *dispatcher()
{
    return &dpfInstance.eventDispatcher();
}

void SideBarEventCaller::sendItemActived(QWidget *sender, const QUrl &url)
{
    quint64 id = SideBarHelper::windowId(sender);
    dispatcher()->publish(GlobalEventType::kChangeCurrentUrl, id, url);
}
