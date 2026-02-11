// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentmanager.h"

#include "recentiterateworker.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QCoreApplication>
#include <QTimer>
#include <QFileInfo>
#include <QFile>

SERVERRECENTMANAGER_BEGIN_NAMESPACE

DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

RecentManager &RecentManager::instance()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    static RecentManager ins;
    return ins;
}

QString RecentManager::xbelPath() const
{
    return QDir::homePath() + "/.local/share/recently-used.xbel";
}

void RecentManager::initialize()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, [this]() {
        RecentIterateWorker *worker = new RecentIterateWorker;
        worker->moveToThread(&workerThread);

        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(qApp, &QCoreApplication::aboutToQuit, this, &RecentManager::finalize);

        // 连接request信号到worker相应的槽函数
        connect(this, &RecentManager::requestReload, worker, &RecentIterateWorker::onRequestReload);
        connect(this, &RecentManager::requestAddRecentItem, worker, &RecentIterateWorker::onRequestAddRecentItem);
        connect(this, &RecentManager::requestRemoveItems, worker, &RecentIterateWorker::onRequestRemoveItems);
        connect(this, &RecentManager::requestPurgeItems, worker, &RecentIterateWorker::onRequestPurgeItems);

        connect(worker, &RecentIterateWorker::reloadFinished, this, &RecentManager::reloadFinished);
        connect(worker, &RecentIterateWorker::purgeFinished, this, &RecentManager::purgeFinished);
        connect(worker, &RecentIterateWorker::itemAdded, this, &RecentManager::onItemAdded);
        connect(worker, &RecentIterateWorker::itemsRemoved, this, &RecentManager::onItemsRemoved);
        connect(worker, &RecentIterateWorker::itemChanged, this, &RecentManager::onItemChanged);

        // 初始化限流定时器
        reloadTimer = new QTimer(this);
        reloadTimer->setSingleShot(true);
        reloadTimer->setInterval(1000);   // 1秒
        connect(reloadTimer, &QTimer::timeout, this, [this]() {
            doReload();
        });

        workerThread.start();
    });
}

void RecentManager::finalize()
{
    static std::once_flag finalizeFlag;
    std::call_once(finalizeFlag, [this]() {
        stopWatch();

        fmDebug() << "[RecentManager::finalize] Recent work finishing...";
        workerThread.quit();
        workerThread.wait(5000);
    });
}

void RecentManager::startWatch()
{
    auto uri { QUrl::fromLocalFile(xbelPath()) };
    QString localPath = uri.toLocalFile();

    if (!QFileInfo(localPath).exists()) {
        // Create empty xbel file if not exists
        QFile file(localPath);
        if (file.open(QIODevice::WriteOnly)) {
            fmInfo() << "[RecentManager::startWatch] Created empty recent file:" << localPath;
            file.close();
        } else {
            fmCritical() << "[RecentManager::startWatch] Failed to create recent file:" << localPath;
            return;
        }
    }

    watcher = WatcherFactory::create<AbstractFileWatcher>(uri);
    fmInfo() << "[RecentManager::startWatch] Starting file watcher for recent file:" << uri;
    // fileAttributeChanged 可能被高频率发送
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentManager::reload, Qt::DirectConnection);
    if (watcher)
        watcher->startWatcher();
    fmInfo() << "[RecentManager::startWatch] File watcher started successfully";
}

void RecentManager::stopWatch()
{
    if (watcher) {
        fmInfo() << "[RecentManager::stopWatch] Stopping file watcher";
        watcher->stopWatcher();
        watcher->disconnect(this);
        fmInfo() << "[RecentManager::stopWatch] File watcher stopped successfully";
    }
}

void RecentManager::reload()
{
    if (!reloadTimer->isActive()) {
        reloadTimer->start();
        return;
    }

    fmWarning() << "[RecentManager::reload] Rejecting reload request due to rate limiting";
}

void RecentManager::doReload(qint64 timestamp)
{
    fmInfo() << "[RecentManager::doReload] Reloading recent file, timestamp:" << timestamp;
    emit requestReload(xbelPath(), timestamp);
}

void RecentManager::forceReload(qint64 timestamp)
{
    fmWarning() << "[RecentManager::forceReload] Force reloading recent file, timestamp:" << timestamp;
    doReload(timestamp);
}

void RecentManager::addRecentItem(const QVariantMap &item)
{
    if (itemsInfo.size() >= kRecentItemLimit) {
        fmWarning() << "[RecentManager::addRecentItem] Recent items exceeded limit:" << kRecentItemLimit
                    << "current count:" << itemsInfo.size();
        return;
    }

    fmDebug() << "[RecentManager::addRecentItem] Adding recent item:" << item.value("path").toString();
    emit requestAddRecentItem(item);
}

void RecentManager::removeItems(const QStringList &hrefs)
{
    fmInfo() << "[RecentManager::removeItems] Removing items:" << hrefs.size() << "items";
    emit requestRemoveItems(hrefs);
}

void RecentManager::purgeItems()
{
    fmInfo() << "[RecentManager::purgeItems] Purging all recent items";
    emit requestPurgeItems(xbelPath());
}

QStringList RecentManager::getItemsPath()
{
    return itemsInfo.keys();
}

QVariantList RecentManager::getItemsInfo()
{
    // 存在一定的性能问题，尽可能少调用该接口
    updateItemsInfoList();
    return itemsInfoList;
}

QVariantMap RecentManager::getItemInfo(const QString &path)
{
    QVariantMap map;
    if (path.isEmpty() || !itemsInfo.contains(path)) {
        fmWarning() << "[RecentManager::getItemInfo] Cannot get item info for path:" << path
                    << "empty:" << path.isEmpty() << "exists:" << itemsInfo.contains(path);
        return map;
    }

    const auto &item { itemsInfo.value(path) };
    map.insert(RecentProperty::kPath, path);
    map.insert(RecentProperty::kHref, item.href);
    map.insert(RecentProperty::kModified, item.modified);
    return map;
}

void RecentManager::onItemAdded(const QString &path, const RecentItem &item)
{
    if (itemsInfo.size() >= kRecentItemLimit) {
        fmWarning() << "[RecentManager::onItemAdded] Recent items exceeded limit:" << kRecentItemLimit
                    << "current count:" << itemsInfo.size() << "path:" << path;
        return;
    }

    fmDebug() << "[RecentManager::onItemAdded] Item added:" << path << "href:" << item.href;
    itemsInfo.insert(path, item);
    emit itemAdded(path, item.href, item.modified);
}

void RecentManager::onItemsRemoved(const QStringList &paths)
{
    fmInfo() << "[RecentManager::onItemsRemoved] Removing items:" << paths.size() << "items";
    for (const QString &path : paths) {
        itemsInfo.remove(path);
    }
    emit itemsRemoved(paths);
}

void RecentManager::onItemChanged(const QString &path, const RecentItem &item)
{
    fmDebug() << "[RecentManager::onItemChanged] Item changed:" << path << "modified:" << item.modified;
    itemsInfo[path] = item;
    emit itemChanged(path, item.modified);
}

void RecentManager::updateItemsInfoList()
{
    itemsInfoList.clear();
    for (auto it = itemsInfo.constBegin(); it != itemsInfo.constEnd(); ++it) {
        const QString &path = it.key();
        const RecentItem &item = it.value();
        QVariantMap map;
        map.insert(RecentProperty::kPath, path);
        map.insert(RecentProperty::kHref, item.href);
        map.insert(RecentProperty::kModified, item.modified);
        itemsInfoList.append(map);
    }
}

RecentManager::RecentManager(QObject *parent)
    : QObject(parent), reloadTimer(nullptr)
{
}

RecentManager::~RecentManager()
{
    finalize();
}

SERVERRECENTMANAGER_END_NAMESPACE
