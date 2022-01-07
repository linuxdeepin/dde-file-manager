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

DPSIDEBAR_USE_NAMESPACE

SideBarManager *SideBarManager::instance()
{
    static SideBarManager manager;
    return &manager;
}

void SideBarManager::runCd(const QString &identifier, quint64 windowId, const QUrl &url)
{
    if (cdCallbackMap.contains(identifier)) {
        Q_ASSERT(cdCallbackMap[identifier]);
        cdCallbackMap[identifier](windowId, url);
    } else {
        SideBarHelper::defaultCdAction(windowId, url);
    }
}

void SideBarManager::runContextMenu(const QString &identifier, quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    if (menuCallbackMap.contains(identifier)) {
        Q_ASSERT(menuCallbackMap[identifier]);
        menuCallbackMap[identifier](windowId, url, globalPos);
    }
}

void SideBarManager::runRename(const QString &identifier, quint64 windowId, const QUrl &url, const QString &name)
{
    if (renameCallbackMap.contains(identifier)) {
        Q_ASSERT(renameCallbackMap[identifier]);
        renameCallbackMap[identifier](windowId, url, name);
    }
}

void SideBarManager::registerCallback(const QString &identifier, const SideBarManager::CdCallback &cd, const SideBarManager::ContextMenuCallback &contexMenu, const SideBarManager::RenameCallback &rename)
{
    if (cd)
        cdCallbackMap.insert(identifier, cd);
    if (contexMenu)
        menuCallbackMap.insert(identifier, contexMenu);
    if (rename)
        renameCallbackMap.insert(identifier, rename);
}

SideBarManager::SideBarManager(QObject *parent)
    : QObject(parent)
{
}

SideBarManager::~SideBarManager()
{
}
