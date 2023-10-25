// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarinfocachemananger.h"

#include <dfm-base/utils/universalutils.h>

DPSIDEBAR_USE_NAMESPACE

SideBarInfoCacheMananger::SideBarInfoCacheMananger()
{
}

QStringList SideBarInfoCacheMananger::getLastSettingBindingKeys() const
{
    return lastSettingBindingKeys;
}

void SideBarInfoCacheMananger::appendLastSettingBindingKey(const QString &value)
{
    if (!lastSettingBindingKeys.contains(value))
        lastSettingBindingKeys.append(value);
}

void SideBarInfoCacheMananger::clearLastSettingBindingKey()
{
    lastSettingBindingKeys.clear();
}

QStringList SideBarInfoCacheMananger::getLastSettingKeys() const
{
    return lastSettingKeys;
}

void SideBarInfoCacheMananger::appendLastSettingKey(const QString &value)
{
    if (!lastSettingKeys.contains(value))
        lastSettingKeys.append(value);
}

void SideBarInfoCacheMananger::clearLastSettingKey()
{
    lastSettingKeys.clear();
}

SideBarInfoCacheMananger *SideBarInfoCacheMananger::instance()
{
    static SideBarInfoCacheMananger instance;
    return &instance;
}

SideBarInfoCacheMananger::GroupList SideBarInfoCacheMananger::groups() const
{
    return cacheInfoMap.keys();
}

SideBarInfoCacheMananger::CacheInfoList SideBarInfoCacheMananger::indexCacheList(const Group &name) const
{
    return cacheInfoMap.value(name);
}

bool SideBarInfoCacheMananger::addItemInfoCache(const ItemInfo &info)
{
    if (contains(info))
        return false;

    CacheInfoList &cache = cacheInfoMap[info.group];
    cache.push_back(info);
    bindedInfos[info.url] = info;

    return true;
}

bool SideBarInfoCacheMananger::insertItemInfoCache(SideBarInfoCacheMananger::Index i, const ItemInfo &info)
{
    if (contains(info))
        return false;

    CacheInfoList &cache = cacheInfoMap[info.group];
    cache.insert(i, info);
    bindedInfos[info.url] = info;

    return true;
}

bool SideBarInfoCacheMananger::removeItemInfoCache(const SideBarInfoCacheMananger::Group &name, const QUrl &url)
{
    CacheInfoList &cache = cacheInfoMap[name];
    int size = cache.size();
    for (int i = 0; i != size; i++) {
        if (DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, cache[i].url)) {
            cache.removeAt(i);
            bindedInfos.remove(url);
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

bool SideBarInfoCacheMananger::updateItemInfoCache(const SideBarInfoCacheMananger::Group &name, const QUrl &url, const ItemInfo &info)
{
    CacheInfoList &cache = cacheInfoMap[name];
    int size = cache.size();
    for (int i = 0; i != size; i++) {
        if (DFMBASE_NAMESPACE::UniversalUtils::urlEquals(url, cache[i].url)) {
            cache[i] = info;
            bindedInfos[url] = info;
            return true;
        }
    }
    return false;
}

bool SideBarInfoCacheMananger::updateItemInfoCache(const QUrl &url, const ItemInfo &info)
{
    bool ret { false };
    GroupList &&allGroup = cacheInfoMap.keys();
    for (const Group &name : allGroup) {
        if (updateItemInfoCache(name, url, info))
            ret = true;
    }

    return ret;
}

ItemInfo SideBarInfoCacheMananger::itemInfo(const QUrl &url)
{
    return bindedInfos.value(url);
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

bool SideBarInfoCacheMananger::contains(const QUrl &url) const
{
    return bindedInfos.contains(url);
}
