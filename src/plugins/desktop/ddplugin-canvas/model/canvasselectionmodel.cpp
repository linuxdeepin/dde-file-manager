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
#include "canvasmodel.h"

DDP_CANVAS_USE_NAMESPACE

CanvasSelectionModel::CanvasSelectionModel(CanvasModel *model, QObject *parent) : QItemSelectionModel(model, parent)
{

}

CanvasModel *CanvasSelectionModel::model()
{
    return qobject_cast<CanvasModel *>(QItemSelectionModel::model());
}

QList<QUrl> CanvasSelectionModel::selectedUrls()
{
    auto indexs = selectedIndexes();
    QList<QUrl> urls;
    for (auto index : indexs) {
        urls <<  model()->url(index);
    }

    return urls;
}
