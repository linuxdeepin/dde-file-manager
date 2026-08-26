// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARINFOCACHEMANANGER_H
#define SIDEBARINFOCACHEMANANGER_H

#include "dfmplugin_sidebar_global.h"

#include <QObject>
#include <QHash>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarInfoCacheMananger
{
    Q_DISABLE_COPY(SideBarInfoCacheMananger)

public:
    using Index = int;
    using Group = QString;
    using GroupList = QStringList;
    using CacheInfoList = QList<ItemInfo>;
    using GroupCacheMap = QHash<Group, CacheInfoList>;

public:
    static SideBarInfoCacheMananger *instance();

    bool contains(const ItemInfo &info) const;
    bool contains(const QUrl &url) const;
    GroupList groups() const;
    CacheInfoList indexCacheList(const Group &name) const;
    ItemInfo itemInfo(const Group &group, const QUrl &url) const;   // 查找指定分组内匹配 URL 的 ItemInfo / Find ItemInfo by URL within a specific group
    ItemInfo itemInfo(const QUrl &url) const;   // 在所有分组中查找匹配 URL 的任一 ItemInfo（顺序未定义）/ Find any matching ItemInfo by URL across all groups (order undefined)

    bool addItemInfoCache(const ItemInfo &info);
    bool insertItemInfoCache(Index i, const ItemInfo &info);
    bool removeItemInfoCache(const Group &name, const QUrl &url);
    bool removeItemInfoCache(const QUrl &url);
    bool updateItemInfoCache(const Group &name, const QUrl &url, const ItemInfo &info);
    bool updateItemInfoCache(const QUrl &url, const ItemInfo &info);

    QStringList getLastSettingKeys() const;
    void appendLastSettingKey(const QString &value);
    void clearLastSettingKey();

    QStringList getLastSettingBindingKeys() const;
    void appendLastSettingBindingKey(const QString &value);
    void clearLastSettingBindingKey();

private:
    SideBarInfoCacheMananger();

private:
    GroupCacheMap cacheInfoMap;
    QStringList lastSettingKeys;
    QStringList lastSettingBindingKeys;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARINFOCACHEMANANGER_H
