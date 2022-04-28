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
#ifndef SIDEBARINFOCACHEMANANGER_H
#define SIDEBARINFOCACHEMANANGER_H

#include "dfmplugin_sidebar_global.h"

#include "services/filemanager/sidebar/sidebar_defines.h"

#include <QObject>
#include <QHash>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarInfoCacheMananger
{
    Q_DISABLE_COPY(SideBarInfoCacheMananger)

public:
    using ItemInfo = DSB_FM_NAMESPACE::SideBar::ItemInfo;
    using Index = int;
    using Group = QString;
    using GroupList = QStringList;
    using CacheInfoList = QList<ItemInfo>;
    using GroupCacheMap = QHash<Group, CacheInfoList>;

public:
    static SideBarInfoCacheMananger *instance();

    bool contains(const ItemInfo &info) const;
    GroupList groups() const;
    CacheInfoList indexCacheMap(const Group &name) const;

    bool addItemInfoCache(const ItemInfo &info);
    bool insertItemInfoCache(Index i, const ItemInfo &info);
    bool removeItemInfoCache(const Group &name, const QUrl &url);
    bool removeItemInfoCache(const QUrl &url);

    bool containsHiddenUrl(const QUrl &url);
    void addHiddenUrl(const QUrl &url);
    void removeHiddenUrl(const QUrl &url);

private:
    SideBarInfoCacheMananger();

private:
    GroupCacheMap cacheInfoMap;
    QList<QUrl> hiddenUrlList;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARINFOCACHEMANANGER_H
