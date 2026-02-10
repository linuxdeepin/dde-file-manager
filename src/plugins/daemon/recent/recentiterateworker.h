// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTITERATEWORKER_H
#define RECENTITERATEWORKER_H

#include "serverplugin_recentmanager_global.h"

#include <DRecentManager>

#include <QObject>
#include <QXmlStreamReader>

SERVERRECENTMANAGER_BEGIN_NAMESPACE

class RecentIterateWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecentIterateWorker(QObject *parent = nullptr);

public Q_SLOTS:
    void onRequestReload(const QString &xbelPath, qint64 timestamp);
    void onRequestAddRecentItem(const QVariantMap &item);
    void onRequestRemoveItems(const QStringList &hrefs);
    void onRequestPurgeItems(const QString &xbelPath);

Q_SIGNALS:
    void reloadFinished(qint64 timestamp);
    void purgeFinished();
    void itemAdded(const QString &path, const RecentItem &item);
    void itemsRemoved(const QStringList &paths);
    void itemChanged(const QString &path, const RecentItem &item);

private:
    void processBookmarkElement(QXmlStreamReader &reader, QStringList &curPathList);
    void removeOutdatedItems(const QStringList &cachedPathList, const QStringList &curPathList);

private:
    QMap<QString, RecentItem> itemsInfo;
};

SERVERRECENTMANAGER_END_NAMESPACE

#endif   // RECENTITERATEWORKER_H
