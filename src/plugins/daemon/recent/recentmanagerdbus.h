// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTMANAGERDBUS_H
#define RECENTMANAGERDBUS_H

#include <QObject>
#include <QStringList>

class RecentManagerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.Filemanager.Daemon.RecentManager")

public:
    explicit RecentManagerDBus(QObject *parent = nullptr);

public Q_SLOTS:
    qint64 Reload();
    void AddItem(const QVariantMap &item);
    void RemoveItems(const QStringList &hrefs);
    void PurgeItems();
    QStringList GetItemsPath();
    QVariantList GetItemsInfo();
    QVariantMap GetItemInfo(const QString &path);

Q_SIGNALS:
    void ReloadFinished(qint64 timestamp);
    void PurgeFinished();
    void ItemAdded(const QString &path, const QString &href, qint64 modified);
    void ItemsRemoved(const QStringList &paths);
    void ItemChanged(const QString &path, qint64 modified);

private:
    void initConnect();
};

#endif   // RECENTMANAGERDBUS_H
