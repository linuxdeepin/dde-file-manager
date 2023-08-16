// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTMANAGER_H
#define RECENTMANAGER_H

#include "dfmplugin_recent_global.h"
#include "files/recentfileinfo.h"
#include "files/recentiterateworker.h"

#include <dfm-base/utils/clipboard.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
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

namespace RecentHelper {
inline QString scheme()
{
    return "recent";
}

inline QIcon icon()
{
    return QIcon::fromTheme("document-open-recent-symbolic");
}

inline QString xbelPath()
{
    return QDir::homePath() + "/.local/share/recently-used.xbel";
}

QUrl rootUrl();
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

    QMap<QUrl, FileInfoPointer> getRecentNodes() const;
    QMap<QUrl, QString> getRecentOriginPaths() const;
    bool removeRecentFile(const QUrl &url);

    bool customColumnRole(const QUrl &rootUrl, QList<DFMGLOBAL_NAMESPACE::ItemRoles> *roleList);
    bool customRoleDisplayName(const QUrl &url, const DFMGLOBAL_NAMESPACE::ItemRoles role, QString *displayName);
    bool detailViewIcon(const QUrl &url, QString *iconName);
    bool sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup);
    bool isTransparent(const QUrl &url, DFMGLOBAL_NAMESPACE::TransparentStatus *status);
    bool checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action);
    bool handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl);

signals:
    void asyncHandleFileChanged(const QList<QUrl> &);

private:
    explicit RecentManager(QObject *parent = nullptr);
    ~RecentManager() override;

    void init();

public slots:
    void updateRecent();
private slots:
    void onUpdateRecentFileInfo(const QUrl &url, const QString &originPath, qint64 readTime);
    void onDeleteExistRecentUrls(const QList<QUrl> &urls);

private:
    QThread workerThread;
    RecentIterateWorker *iteratorWorker { new RecentIterateWorker };   // free by QThread::finished
    AbstractFileWatcherPointer watcher;
    dfmbase::DThreadMap<QUrl, FileInfoPointer> recentNodes;
    QMap<QUrl, QString> recentOriginPaths;
};
}
#endif   // RECENTMANAGER_H
