// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDATAMANAGER_H
#define FILEDATAMANAGER_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/traversaldirthread.h>
#include <dfm-base/base/application/application.h>

#include <QObject>
#include <QHash>
#include <QMultiHash>
#include <QSet>

namespace dfmplugin_workspace {

class RootInfo;
class FileItemData;
class FileDataManager : public QObject
{
    Q_OBJECT
public:
    static FileDataManager *instance();

    // NOTE: do not call this from delegate paint/sizeHint paths.
    RootInfo *fetchRoot(const QUrl &url, const QString &key = QString());

    bool fetchFiles(const QUrl &rootUrl,
                    const QString &key,
                    DFMGLOBAL_NAMESPACE::ItemRoles role = DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole,
                    Qt::SortOrder order = Qt::AscendingOrder);
    // self = false, will clean children
    void cleanRoot(const QUrl &rootUrl, const QString &key, const bool refresh = false, const bool self = true);
    void cleanRoot(const QUrl &rootUrl);
    // 清理不再需要的RootInfo对象，保留当前URL和其直接子目录的RootInfo
    void cleanUnusedRoots(const QUrl &currentUrl, const QString &key);
    void stopRootWork(const QUrl &rootUrl, const QString &key);
    void setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active);

public Q_SLOTS:
    void onAppAttributeChanged(DFMBASE_NAMESPACE::Application::ApplicationAttribute aa, const QVariant &value);
    void onHandleFileDeleted(const QUrl url);

private:
    explicit FileDataManager(QObject *parent = nullptr);
    ~FileDataManager();

    RootInfo *createRoot(const QUrl &url);
    QUrl normalizeRootUrl(const QUrl &url) const;
    void addRootUser(const QUrl &url, const QString &key);
    void removeRootUser(const QUrl &url, const QString &key);
    bool hasRootUsers(const QUrl &url) const;
    void handleDeletion(RootInfo *root);

    QMap<QUrl, RootInfo *> rootInfoMap {};
    QMap<QUrl, TraversalThreadPointer> traversalPointerMap {};

    bool isMixFileAndFolder { false };

    QHash<QUrl, QSet<QString>> rootUsers {};
    QList<RootInfo *> deleteLaterList {};
};

}

#endif   // FILEDATAMANAGER_H
