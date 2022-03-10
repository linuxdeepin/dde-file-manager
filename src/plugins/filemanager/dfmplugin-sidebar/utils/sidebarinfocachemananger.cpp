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
#include "sidebarinfocachemananger.h"

#include "dfm-base/utils/universalutils.h"

DPSIDEBAR_USE_NAMESPACE

SideBarInfoCacheMananger::SideBarInfoCacheMananger()
{
}

SideBarInfoCacheMananger *SideBarInfoCacheMananger::instance()
{
    static SideBarInfoCacheMananger instance;
    return &instance;
}

SideBarInfoCacheMananger::GroupList SideBarInfoCacheMananger::goups() const
{
    return cacheInfoMap.keys();
}

SideBarInfoCacheMananger::CacheInfoList SideBarInfoCacheMananger::indexCacheMap(const Group &name) const
{
    return cacheInfoMap.value(name);
}

bool SideBarInfoCacheMananger::addItemInfoCache(const ItemInfo &info)
{
    if (contains(info))
        return false;

    CacheInfoList &cache = cacheInfoMap[info.group];
    cache.push_back(info);

    return true;
}

bool SideBarInfoCacheMananger::insertItemInfoCache(SideBarInfoCacheMananger::Index i, const ItemInfo &info)
{
    if (contains(info))
        return false;

    CacheInfoList &cache = cacheInfoMap[info.group];
    cache.insert(i, info);

    return true;
}

bool SideBarInfoCacheMananger::removeItemInfoCache(const SideBarInfoCacheMananger::Group &name, const QUrl &url)
{
    CacheInfoList &cache = cacheInfoMap[name];
    int size = cache.size();
    for (int i = 0; i != size; i++) {
        if (DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, cache[i].url)) {
            cache.removeAt(i);
            return true;
        }
    }
    return false;
}

bool SideBarInfoCacheMananger::removeItemInfoCache(const QUrl &url)
{
    bool ret { false };
    GroupList &&allGroup = cacheInfoMap.keys();
    for (const Group &name : allGroup) {
        if (removeItemInfoCache(name, url))
            ret = true;
    }

    return ret;
}

bool SideBarInfoCacheMananger::contains(const ItemInfo &info) const
{
    const CacheInfoList &cache = cacheInfoMap.value(info.group);
    int size = cache.size();
    for (int i = 0; i != size; i++) {
        if (cache[i] == info)
            return true;
    }
    return false;
}
