// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTMANAGER_H
#define RECENTMANAGER_H

#include "dfmplugin_recent_global.h"
#include "recentmanager_interface.h"
#include "files/recentfileinfo.h"

#include <dfm-base/utils/clipboard.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/threadcontainer.h>

#include <QUrl>
#include <QDebug>
#include <QFile>
#include <QQueue>
#include <QThread>
#include <QTimer>
#include <QDir>

namespace dfmplugin_recent {

using BasicExpand = QMultiMap<QString, QPair<QString, QString>>;
using ExpandFieldMap = QMap<QString, BasicExpand>;
using RecentManagerDBusInterface = OrgDeepinFilemanagerDaemonRecentManagerInterface;

namespace RecentHelper {
inline QString scheme()
{
    return "recent";
}

inline QIcon icon()
{
    return QIcon::fromTheme("document-open-recent-symbolic");
}

QUrl rootUrl();
QUrl recentUrl(const QString &path);
void removeRecent(const QList<QUrl> &urls);
void clearRecent();
bool openFileLocation(const QUrl &url);
void openFileLocation(const QList<QUrl> &urls);
void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
ExpandFieldMap propetyExtensionFunc(const QUrl &url);
QUrl urlTransform(const QUrl &url);
}   // namespace RecentHelper

class RecentManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RecentManager)

public:
    static RecentManager *instance();
    RecentManagerDBusInterface *dbus() const;
    void init();

    QMap<QUrl, FileInfoPointer> getRecentNodes() const;
    QString getRecentOriginPaths(const QUrl &url) const;
    bool removeRecentFile(const QUrl &url);
    int size();

private:
    explicit RecentManager(QObject *parent = nullptr);
    ~RecentManager() override;
    void resetRecentNodes();

public slots:
    void reloadRecent();
    void onItemAdded(const QString &path, const QString &href, qint64 modified);
    void onItemsRemoved(const QStringList &paths);
    void onItemChanged(const QString &path, qint64 modified);

private:
    QScopedPointer<RecentManagerDBusInterface> recentDBusInterce;
    struct RecentItem
    {
        FileInfoPointer fileInfo;
        QString originPath;
    };
    QMap<QUrl, RecentItem> recentItems;
};
}   // namespace dfmplugin_recent
#endif   // RECENTMANAGER_H
