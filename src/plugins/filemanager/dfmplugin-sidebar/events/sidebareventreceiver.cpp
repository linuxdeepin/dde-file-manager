/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "sidebareventreceiver.h"
#include "views/sidebarwidget.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "services/filemanager/windows/windowsservice.h"

#include <dfm-framework/dpf.h>

DPSIDEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

SideBarEventReceiver *SideBarEventReceiver::instance()
{
    static SideBarEventReceiver ins;
    return &ins;
}

void SideBarEventReceiver::connectService()
{
    static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPSIDEBAR_NAMESPACE) };
    // TODO(zhangs): use slot event
    dpfSignalDispatcher->subscribe(SideBar::EventType::kItemVisibleSetting, this, &SideBarEventReceiver::handleItemVisibleSetting);

    dpfSlotChannel->connect(kCurrentEventSpace, "slot_SetContextMenuEnable", this, &SideBarEventReceiver::handleSetContextMenuEnable);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_GetGroupItems", this, &SideBarEventReceiver::handleGetGroupItems);
}

void SideBarEventReceiver::handleItemVisibleSetting(const QUrl &url, bool visible)
{
    if (visible)
        SideBarInfoCacheMananger::instance()->removeHiddenUrl(url);
    else
        SideBarInfoCacheMananger::instance()->addHiddenUrl(url);

    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->setItemVisible(url, visible);
}

void SideBarEventReceiver::handleSetContextMenuEnable(bool enable)
{
    SideBarHelper::contextMenuEnabled = enable;
}

QList<QUrl> SideBarEventReceiver::handleGetGroupItems(quint64 winId, const QString &group)
{
    if (group.isEmpty())
        return {};

    SideBarWidget *wid { nullptr };
    for (auto sb : SideBarHelper::allSideBar()) {
        if (WindowsService::service()->findWindowId(sb) == winId) {
            wid = sb;
            break;
        }
    }

    if (wid)
        return wid->findItems(group);

    qDebug() << "cannot find sidebarwidget for winid: " << winId << group;
    return {};
}

SideBarEventReceiver::SideBarEventReceiver(QObject *parent)
    : QObject(parent)
{
}
