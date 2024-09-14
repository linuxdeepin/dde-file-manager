// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarmanager.h"
#include "utils/sidebarhelper.h"
#include "sidebaritem.h"

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

    SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);

    if (!item || separatorItem)
        return;

    auto url = item->url();
    auto info = item->itemInfo();
    if (info.contextMenuCb) {
        info.contextMenuCb(windowId, url, globalPos);
    } else {
        SideBarHelper::defaultContextMenu(windowId, url, globalPos);
    }
}

void SideBarManager::runRename(SideBarItem *item, quint64 windowId, const QString &name)
{
    if (!item)
        return;

    auto url = item->url();
    auto info = item->itemInfo();
    if (info.renameCb) {
        info.renameCb(windowId, url, name);
    }
}

void SideBarManager::openFolderInASeparateProcess(const QUrl &url)
{
    SideBarHelper::openFolderInASeparateProcess(url);
}

SideBarManager::SideBarManager(QObject *parent)
    : QObject(parent)
{
}

SideBarManager::~SideBarManager()
{
}
