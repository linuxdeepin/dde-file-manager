// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasselectionmodel.h"
#include "canvasproxymodel.h"

#include <QDebug>

using namespace ddplugin_canvas;

CanvasSelectionModel::CanvasSelectionModel(CanvasProxyModel *model, QObject *parent) : QItemSelectionModel(model, parent)
{
    // clear immediately the cache if selection changed.
    connect(this, &CanvasSelectionModel::selectionChanged, this, &CanvasSelectionModel::clearSelectedCache, Qt::DirectConnection);
}

CanvasProxyModel *CanvasSelectionModel::model() const
{
    return qobject_cast<CanvasProxyModel *>(const_cast<QAbstractItemModel *>(QItemSelectionModel::model()));
}

QModelIndexList CanvasSelectionModel::selectedIndexesCache() const
{
    if (selectedCache.isEmpty()) {
        selectedCache = selectedIndexes();
    }

    return selectedCache;
}

void CanvasSelectionModel::clearSelectedCache()
{
    selectedCache.clear();
}

QList<QUrl> CanvasSelectionModel::selectedUrls() const
{
    auto indexs = selectedIndexesCache();
    QList<QUrl> urls;
    for (auto index : indexs)
        urls <<  model()->fileUrl(index);

    return urls;
}
