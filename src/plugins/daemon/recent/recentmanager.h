// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTMANAGER_H
#define RECENTMANAGER_H

#include "serverplugin_recentmanager_global.h"

#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <DRecentManager>

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QDir>
#include <QCoreApplication>

DFMBASE_USE_NAMESPACE

SERVERRECENTMANAGER_BEGIN_NAMESPACE

class RecentManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RecentManager)

public:
    static RecentManager &instance();

    void forceReload(qint64 timestamp);
    void addRecentItem(const QVariantMap &items);
    void removeItems(const QStringList &hrefs);
    void purgeItems();
    QStringList getItemsPath();
    QVariantList getItemsInfo();
    QVariantMap getItemInfo(const QString &path);

Q_SIGNALS:
    void requestReload(const QString &xbelPath, qint64 timestamp);
    void requestAddRecentItem(const QVariantMap &items);
    void requestRemoveItems(const QStringList &paths);
    void requestPurgeItems(const QString &xbelPath);

    void reloadFinished(qint64 timestamp);
    void purgeFinished();
    void itemAdded(const QString &path, const QString &href, qint64 modified);
    void itemsRemoved(const QStringList &paths);
    void itemChanged(const QString &path, qint64 modified);

public Q_SLOTS:
    void initialize();
    void finalize();
    void startWatch();
    void stopWatch();

private Q_SLOTS:
    void reload();
    void doReload(qint64 timestamp = 0);
    void onItemAdded(const QString &path, const RecentItem &item);
    void onItemsRemoved(const QStringList &paths);
    void onItemChanged(const QString &path, const RecentItem &item);

private:
    explicit RecentManager(QObject *parent = nullptr);
    ~RecentManager() override;
    QString xbelPath() const;
    void updateItemsInfoList();

private:
    QThread workerThread;
    AbstractFileWatcherPointer watcher;
    QTimer *reloadTimer { nullptr };
    QMap<QString, RecentItem> itemsInfo;
    QVariantList itemsInfoList;
};

SERVERRECENTMANAGER_END_NAMESPACE

#endif   // RECENTMANAGER_H
