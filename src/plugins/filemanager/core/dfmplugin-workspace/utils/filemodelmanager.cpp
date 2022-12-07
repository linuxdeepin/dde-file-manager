/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "filemodelmanager.h"

#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"

#include "dfm-base/dfm_global_defines.h"

using namespace dfmplugin_workspace;

FileModelManager *FileModelManager::instance()
{
    static FileModelManager ins;
    return &ins;
}

FileSortFilterProxyModel *FileModelManager::createProxyModel(QAbstractItemView *view)
{
    FileSortFilterProxyModel *proxyModel = new FileSortFilterProxyModel(view);
    proxyModel->setSourceModel(sourceDataModel);

    connect(sourceDataModel, &FileViewModel::traverPrehandle, proxyModel, &FileSortFilterProxyModel::onTraverPrehandle);
    connect(sourceDataModel, &FileViewModel::stateChanged, proxyModel, &FileSortFilterProxyModel::onStateChanged);
    connect(sourceDataModel, &FileViewModel::childrenUpdated, proxyModel, &FileSortFilterProxyModel::onChildrenUpdate);
    connect(sourceDataModel, &FileViewModel::selectAndEditFile, proxyModel, &FileSortFilterProxyModel::onSelectAndEditFile);

    return proxyModel;
}

void FileModelManager::refRootData(const QUrl &url)
{
    if (needCacheData(url.scheme()))
        return;

    int refCount = 1;
    if (dataRefMap.contains(url))
        refCount = dataRefMap[url] + 1;

    dataRefMap[url] = refCount;
}

void FileModelManager::derefRootData(const QUrl &url)
{
    if (needCacheData(url.scheme()))
        return;

    if (dataRefMap.contains(url)) {
        int refCount = dataRefMap[url];
        if (--refCount <= 0) {
            //do clean cache data
            sourceDataModel->cleanDataCacheByUrl(url);

            dataRefMap.remove(url);
            return;
        }

        dataRefMap[url] = refCount;
    }
}

void FileModelManager::registerDataCache(const QString &scheme)
{
    if (!cacheDataSchemes.contains(scheme))
        cacheDataSchemes.append(scheme);
}

bool FileModelManager::needCacheData(const QString &scheme)
{
    return cacheDataSchemes.contains(scheme);
}

FileModelManager::FileModelManager(QObject *parent)
    : QObject(parent),
      sourceDataModel(new FileViewModel)
{
    cacheDataSchemes.append(DFMGLOBAL_NAMESPACE::Scheme::kFile);
}
