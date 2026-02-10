// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentmanagerdbus.h"

#include "recentmanager.h"

#include <QDebug>
#include <QDateTime>

SERVERRECENTMANAGER_USE_NAMESPACE

RecentManagerDBus::RecentManagerDBus(QObject *parent)
    : QObject(parent)
{
    initConnect();
}

void RecentManagerDBus::initConnect()
{
    connect(&RecentManager::instance(), &RecentManager::reloadFinished, this, &RecentManagerDBus::ReloadFinished);
    connect(&RecentManager::instance(), &RecentManager::purgeFinished, this, &RecentManagerDBus::PurgeFinished);
    connect(&RecentManager::instance(), &RecentManager::itemAdded, this, &RecentManagerDBus::ItemAdded);
    connect(&RecentManager::instance(), &RecentManager::itemsRemoved, this, &RecentManagerDBus::ItemsRemoved);
    connect(&RecentManager::instance(), &RecentManager::itemChanged, this, &RecentManagerDBus::ItemChanged);
}

// Reload recent items and return the timestamp of the operation
qint64 RecentManagerDBus::Reload()
{
    auto timestamp { QDateTime::currentMSecsSinceEpoch() };
    fmInfo() << "[RecentManagerDBus::Reload] Force reloading recent items, timestamp:" << timestamp;
    // 强制重新解析 recent 文件，如果频繁调用此接口，将导致极大的性能开销
    RecentManager::instance().forceReload(timestamp);
    return timestamp;
}

void RecentManagerDBus::AddItem(const QVariantMap &item)
{
    fmDebug() << "[RecentManagerDBus::AddItem] Adding recent item via DBus:" << item.value("path").toString();
    RecentManager::instance().addRecentItem(item);
}

// Remove specified items from the recent list
void RecentManagerDBus::RemoveItems(const QStringList &hrefs)
{
    fmInfo() << "[RecentManagerDBus::RemoveItems] Removing items via DBus:" << hrefs.size() << "items";
    RecentManager::instance().removeItems(hrefs);
}

// Clear all items from the recent list
void RecentManagerDBus::PurgeItems()
{
    fmInfo() << "[RecentManagerDBus::PurgeItems] Purging all items via DBus";
    RecentManager::instance().purgeItems();
}

// Get paths of all recent items
QStringList RecentManagerDBus::GetItemsPath()
{
    return RecentManager::instance().getItemsPath();
}

// Get detailed information of all recent items
QVariantList RecentManagerDBus::GetItemsInfo()
{
    return RecentManager::instance().getItemsInfo();
}

// Get detailed information of a specific recent item
QVariantMap RecentManagerDBus::GetItemInfo(const QString &path)
{
    return RecentManager::instance().getItemInfo(path);
}
