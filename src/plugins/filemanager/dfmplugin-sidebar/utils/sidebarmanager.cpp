// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarmanager.h"
#include "utils/sidebarhelper.h"
#include "treeviews/sidebaritem.h"

#include <dfm-framework/event/event.h>

DPSIDEBAR_USE_NAMESPACE

SideBarManager *SideBarManager::instance()
{
    static SideBarManager manager;
    return &manager;
}

void SideBarManager::runCd(SideBarItem *item, quint64 windowId)
{
    if (!item) {
        fmWarning() << "Cannot run cd operation, item is null";
        return;
    }

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
    if (!SideBarHelper::contextMenuEnabled) {
        fmDebug() << "Context menu is disabled";
        return;
    }

    SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);

    if (!item || separatorItem) {
        fmWarning() << "Cannot show context menu, invalid item or separator item";
        return;
    }

    auto url = item->url();
    if (!url.isValid()) {
        fmWarning() << "Cannot show context menu, invalid URL:" << url;
        return;
    }

    auto info = item->itemInfo();
    if (info.contextMenuCb) {
        info.contextMenuCb(windowId, url, globalPos);
    } else {
        SideBarHelper::defaultContextMenu(windowId, url, globalPos);
    }
}

void SideBarManager::runRename(SideBarItem *item, quint64 windowId, const QString &name)
{
    if (!item) {
        fmWarning() << "Cannot run rename operation, item is null";
        return;
    }

    auto url = item->url();
    auto info = item->itemInfo();
    if (info.renameCb) {
        info.renameCb(windowId, url, name);
    } else {
        fmWarning() << "No rename callback available for item:" << url;
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
