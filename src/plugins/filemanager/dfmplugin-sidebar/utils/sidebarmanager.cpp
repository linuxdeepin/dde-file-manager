/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "sidebarmanager.h"
#include "utils/sidebarhelper.h"
#include "views/sidebaritem.h"

#include <dfm-framework/event/event.h>

DPSIDEBAR_USE_NAMESPACE

SideBarManager *SideBarManager::instance()
{
    static SideBarManager manager;
    return &manager;
}

void SideBarManager::runCd(SideBarItem *item, quint64 windowId)
{
    if (!item)
        return;

    auto url = item->url();
    auto info = item->itemInfo();

    if (info.clickedCb) {
        info.clickedCb(windowId, url);
    } else {
        SideBarHelper::defaultCdAction(windowId, url);
    }
}

void SideBarManager::runContextMenu(SideBarItem *item, quint64 windowId, const QPoint &globalPos)
{
    if (!SideBarHelper::contextMenuEnabled)
        return;

    if (!item)
        return;

    auto url = item->url();
    auto info = item->itemInfo();
    if (info.contextMenuCb) {
        info.contextMenuCb(windowId, url, globalPos);
    } else {
        SideBarHelper::defaultContenxtMenu(windowId, url, globalPos);
    }
}

void SideBarManager::runRename(SideBarItem *item, quint64 windowId, const QString &name)
{
    if (!item)
        return;

    auto url = item->url();
    auto info = item->itemInfo();
    if (info.contextMenuCb) {
        info.renameCb(windowId, url, name);
    }
}

SideBarManager::SideBarManager(QObject *parent)
    : QObject(parent)
{
}

SideBarManager::~SideBarManager()
{
}
