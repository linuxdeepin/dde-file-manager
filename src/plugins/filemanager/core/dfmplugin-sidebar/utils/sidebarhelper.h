// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARHELPER_H
#define SIDEBARHELPER_H

#include "dfmplugin_sidebar_global.h"

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
    static void addSideBar(quint64 windowId, SideBarWidget *sideBar);
    static void removeSideBar(quint64 windowId);
    static quint64 windowId(QWidget *sender);
    static SideBarItem *createItemByInfo(const ItemInfo &info);
    static SideBarItemSeparator *createSeparatorItem(const QString &group);
    static QString makeItemIdentifier(const QString &group, const QUrl &url);
    static void defaultCdAction(quint64 windowId, const QUrl &url);
    static void defaultContextMenu(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static bool registerSortFunc(const QString &subGroup, SortFunc func);
    static SortFunc sortFunc(const QString &subGroup);
    static void updateSideBarSelection(quint64 winId);

    static void bindSettings();
    static QVariantMap hiddenRules();
    static QVariantMap groupExpandRules();

    static void saveGroupsStateToConfig(const QVariant &var);

public:
    static bool contextMenuEnabled;   // TODO(xust) tmp solution, using GroupPolicy instead.

private:
    static QMutex &mutex();
    static QMap<quint64, SideBarWidget *> kSideBarMap;
    static QMap<QString, SortFunc> kSortFuncs;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARHELPER_H
