// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasselectionmodel.h"
#include "canvasproxymodel.h"
#include "selectionhookinterface.h"

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

void CanvasSelectionModel::clear()
{
    // 检查是否是由collectview的SelectionSyncHelper触发的清除
    bool isSyncClearing = property("syncHelperClearing").toBool();

    // 一般的，在canvasview存在多选的时候，在collectview中点击选择会触发canvasselectionmodel的clear
    // 从而导致hook->clear()被调用，进一步collectionview的selectionModel()->clear()被调用，就会存在collectview不能设置选中文件
    // 所以需要区分开来，如果是syncHelper触发的清除，则不调用hook->clear()
    if (!isSyncClearing && hook)
        hook->clear();

    QItemSelectionModel::clear();

    // 部分情况下（如刷新），QItemSelectionModel::clear不会触发selectionChanged信号，
    // 从而导致selectedCache未被清空，所以需要手动清空
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

void CanvasSelectionModel::selectAll()
{
    auto m = model();
    const int row = m->rowCount(m->rootIndex());
    if (row < 1)
        return;

    QItemSelection allIndex(m->index(0, 0), m->index(row - 1, 0));
    select(allIndex, QItemSelectionModel::ClearAndSelect);
}

void CanvasSelectionModel::hookClear()
{
    if (hook)
        hook->clear();
}


