// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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

        fmDebug() << "Recent work fnishing..";
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
            fmInfo() << "Created empty recent file:" << localPath;
            file.close();
        } else {
            fmWarning() << "Failed to create recent file:" << localPath;
            return;
        }
    }

    watcher = WatcherFactory::create<AbstractFileWatcher>(uri);
    fmDebug() << "Start watch recent file: " << uri;
    // fileAttributeChanged 可能被高频率发送
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentManager::reload, Qt::DirectConnection);
    watcher->startWatcher();
}

void RecentManager::stopWatch()
{
    if (watcher) {
        watcher->stopWatcher();
        watcher->disconnect(this);
    }
}

void RecentManager::reload()
{
    if (!reloadTimer->isActive()) {
        reloadTimer->start();
        return;
    }

    fmWarning() << "Reject reload, because it's too often";
}

void RecentManager::doReload(qint64 timestamp)
{
    fmInfo() << "Reaload recent file, timestamp:" << timestamp;
    emit requestReload(xbelPath(), timestamp);
}

void RecentManager::forceReload(qint64 timestamp)
{
    fmWarning() << "Force reaload recent file!" << timestamp;
    doReload(timestamp);
}

void RecentManager::addRecentItem(const QVariantMap &item)
{
    if (itemsInfo.size() >= kRecentItemLimit) {
        fmWarning() << "Recent Item exceeded the limit: " << kRecentItemLimit;
        return;
    }

    emit requestAddRecentItem(item);
}

void RecentManager::removeItems(const QStringList &hrefs)
{
    emit requestRemoveItems(hrefs);
}

void RecentManager::purgeItems()
{
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
        fmWarning() << "Cannot get item info: " << path;
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
        fmWarning() << "Recent Item exceeded the limit: " << kRecentItemLimit;
        return;
    }

    itemsInfo.insert(path, item);
    emit itemAdded(path, item.href, item.modified);
}

void RecentManager::onItemsRemoved(const QStringList &paths)
{
    for (const QString &path : paths) {
        itemsInfo.remove(path);
    }
    emit itemsRemoved(paths);
}

void RecentManager::onItemChanged(const QString &path, const RecentItem &item)
{
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
