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

namespace dfmplugin_workspace {

class RootInfo;
class FileItemData;
class FileDataManager : public QObject
{
    Q_OBJECT
public:
    static FileDataManager *instance();

    RootInfo *fetchRoot(const QUrl &url);

    bool fetchFiles(const QUrl &rootUrl,
                    const QString &key,
                    DFMGLOBAL_NAMESPACE::ItemRoles role = DFMGLOBAL_NAMESPACE::kItemFileDisplayNameRole,
                    Qt::SortOrder order = Qt::AscendingOrder);
    // self = false, will clean children
    void cleanRoot(const QUrl &rootUrl, const QString &key, const bool refresh = false, const bool self = true);
    void cleanRoot(const QUrl &rootUrl);
    void setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active);

public Q_SLOTS:
    void onAppAttributeChanged(DFMBASE_NAMESPACE::Application::ApplicationAttribute aa, const QVariant &value);
    void onHandleFileDeleted(const QUrl url);

private:
    explicit FileDataManager(QObject *parent = nullptr);
    ~FileDataManager();

    RootInfo *createRoot(const QUrl &url);

    bool checkNeedCache(const QUrl &url);

    QMap<QUrl, RootInfo *> rootInfoMap {};
    QMap<QUrl, TraversalThreadPointer> traversalPointerMap {};

    bool isMixFileAndFolder { false };

    // scheme in cacheDataSchemes will have cache
    QList<QString> cacheDataSchemes {};
    QMap<QUrl, int> dataRefMap {};
};

}

#endif   // FILEDATAMANAGER_H
