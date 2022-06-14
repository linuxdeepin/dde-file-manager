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
#ifndef SIDEBARHELPER_H
#define SIDEBARHELPER_H

#include "dfmplugin_sidebar_global.h"

#include "services/filemanager/sidebar/sidebar_defines.h"

#include <QMap>
#include <QMutex>
#include <QWidget>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarWidget;
class SideBarItem;
class SideBarItemSeparator;
class SideBarHelper
{
public:
    static QList<SideBarWidget *> allSideBar();
    static SideBarWidget *findSideBarByWindowId(quint64 windowId);
    static void addSideBar(quint64 windowId, SideBarWidget *titleBar);
    static void removeSideBar(quint64 windowId);
    static quint64 windowId(QWidget *sender);
    static SideBarItem *createDefaultItem(const QString &pathKey, const QString &group);
    static SideBarItem *createItemByInfo(const DSB_FM_NAMESPACE::SideBar::ItemInfo &info);
    static SideBarItemSeparator *createSeparatorItem(const QString &group);
    static QString makeItemIdentifier(const QString &group, const QUrl &url);
    static void defaultCdAction(quint64 windowId, const QUrl &url);
    static void defaultContenxtMenu(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static bool registerSortFunc(const QString &subGroup, DSB_FM_NAMESPACE::SideBar::SortFunc func);
    static DSB_FM_NAMESPACE::SideBar::SortFunc sortFunc(const QString &subGroup);
    static void updateSideBarSelection(quint64 winId);

public:
    static bool contextMenuEnabled;   // TODO(xust) tmp solution, using GroupPolicy instead.

private:
    static QMutex &mutex();
    static QMap<quint64, SideBarWidget *> kSideBarMap;
    static QMap<QString, DSB_FM_NAMESPACE::SideBar::SortFunc> kSortFuncs;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARHELPER_H
