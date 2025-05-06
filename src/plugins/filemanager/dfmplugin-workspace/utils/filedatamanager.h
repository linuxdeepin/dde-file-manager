// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDATAMANAGER_H
#define FILEDATAMANAGER_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/traversaldirthread.h>
#include <dfm-base/base/application/application.h>

#include <QObject>
#include <QMultiHash>

namespace dfmplugin_workspace {

class RootInfo;
class FileItemData;
class FileDataManager : public QObject
{
    Q_OBJECT
public:
    static FileDataManager *instance();

    void initMntedDevsCache();
    RootInfo *fetchRoot(const QUrl &url);

    bool fetchFiles(const QUrl &rootUrl,
                    const QString &key,
                    DFMGLOBAL_NAMESPACE::ItemRoles role = DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole,
                    Qt::SortOrder order = Qt::AscendingOrder);
    // self = false, will clean children
    void cleanRoot(const QUrl &rootUrl, const QString &key, const bool refresh = false, const bool self = true);
    void cleanRoot(const QUrl &rootUrl);
    void stopRootWork(const QUrl &rootUrl, const QString &key);
    void setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active);

public Q_SLOTS:
    void onAppAttributeChanged(DFMBASE_NAMESPACE::Application::ApplicationAttribute aa, const QVariant &value);
    void onHandleFileDeleted(const QUrl url);
    void removeCachedMnts(const QString &id);

private:
    explicit FileDataManager(QObject *parent = nullptr);
    ~FileDataManager();

    RootInfo *createRoot(const QUrl &url);
    bool checkNeedCache(const QUrl &url);
    void handleDeletion(RootInfo *root);

    QMap<QUrl, RootInfo *> rootInfoMap {};
    QMap<QUrl, TraversalThreadPointer> traversalPointerMap {};

    bool isMixFileAndFolder { false };

    // scheme in cacheDataSchemes will have cache
    QList<QString> cacheDataSchemes {};
    QMap<QUrl, int> dataRefMap {};
    QList<RootInfo *> deleteLaterList {};
};

}

#endif   // FILEDATAMANAGER_H
