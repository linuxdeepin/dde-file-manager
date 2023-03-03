// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMODELMANAGER_H
#define FILEMODELMANAGER_H

#include "dfmplugin_workspace_global.h"

#include <QAbstractItemView>

namespace dfmplugin_workspace {

class FileViewModel;
class FileSortFilterProxyModel;
class FileModelManager : public QObject
{
    Q_OBJECT
public:
    static FileModelManager *instance();
    FileSortFilterProxyModel *createProxyModel(QAbstractItemView *view);

    void refRootData(const QUrl &url);
    void derefRootData(const QUrl &url);

    bool needCacheData(const QString &scheme);
    void registerDataCache(const QString &scheme);

private:
    explicit FileModelManager(QObject *parent = nullptr);

    FileViewModel *sourceDataModel = nullptr;

    // scheme in cacheDataSchemes will cache data in source model
    QList<QString> cacheDataSchemes {};
    QMap<QUrl, int> dataRefMap {};
};

}

#endif   // FILEMODELMANAGER_H
