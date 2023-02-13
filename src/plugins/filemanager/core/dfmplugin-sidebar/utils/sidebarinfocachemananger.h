// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
    CacheInfoList indexCacheMap(const Group &name) const;
    bool containsHiddenUrl(const QUrl &url);
    ItemInfo itemInfo(const QUrl &url);   // the funcs is for QHash<QUrl, ItemInfo> bindedInfos;
    QList<QUrl> findItems(const QString &visiableKey);

    bool addItemInfoCache(const ItemInfo &info);
    bool insertItemInfoCache(Index i, const ItemInfo &info);
    bool removeItemInfoCache(const Group &name, const QUrl &url);
    bool removeItemInfoCache(const QUrl &url);
    bool updateItemInfoCache(const Group &name, const QUrl &url, const ItemInfo &info);
    bool updateItemInfoCache(const QUrl &url, const ItemInfo &info);

    void addHiddenUrl(const QUrl &url);   // TODO(zhangs): update
    void removeHiddenUrl(const QUrl &url);   // TODO(zhangs): update

private:
    SideBarInfoCacheMananger();

private:
    GroupCacheMap cacheInfoMap;
    QList<QUrl> hiddenUrlList;
    QHash<QUrl, ItemInfo> bindedInfos;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARINFOCACHEMANANGER_H
