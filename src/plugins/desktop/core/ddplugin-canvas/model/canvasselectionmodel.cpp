/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
    if (selectedCahe.isEmpty()) {
        selectedCahe = selectedIndexes();
    }

    return selectedCahe;
}

void CanvasSelectionModel::clearSelectedCache()
{
    selectedCahe.clear();
}

QList<QUrl> CanvasSelectionModel::selectedUrls() const
{
    auto indexs = selectedIndexesCache();
    QList<QUrl> urls;
    for (auto index : indexs)
        urls <<  model()->fileUrl(index);

    return urls;
}
