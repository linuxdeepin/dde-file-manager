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
    // TODO: remove follow interfaces
    static QList<SideBarWidget *> allSideBar();
    static void addSideBar(quint64 windowId, SideBarWidget *sideBar);
    static void removeSideBar(quint64 windowId);

    // business
    static quint64 windowId(QWidget *sender);
    static QMap<QUrl, QPair<int, QVariantMap>> preDefineItemProperties();
    static SideBarItem *createItemByInfo(const ItemInfo &info);
    static SideBarItemSeparator *createSeparatorItem(const QString &group);
    static QString makeItemIdentifier(const QString &group, const QUrl &url);
    static void defaultCdAction(quint64 windowId, const QUrl &url);
    static void defaultContextMenu(quint64 windowId, const QUrl &url, const QPoint &globalPos);

    // for settings
    static void bindSetting(const QString &itemVisiableSettingKey, const QString &itemVisiableControlKey);
    static void removebindingSetting(const QString &itemVisiableSettingKey);
    static void initDefaultSettingPanel();
    static void addItemToSettingPannel(const QString &group, const QString &key, const QString &value, QMap<QString, int> *levelMap);
    static void removeItemFromSetting(const QString &key);
    static void registCustomSettingItem();
    static QVariantMap hiddenRules();
    static QVariantMap groupExpandRules();

    static void saveGroupsStateToConfig(const QVariant &var);
    static void openFolderInASeparateProcess(const QUrl &url);

public:
    static bool contextMenuEnabled;   // TODO(xust) tmp solution, using GroupPolicy instead.

private:
    static QMutex &mutex();
    static QMap<quint64, SideBarWidget *> kSideBarMap;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARHELPER_H
