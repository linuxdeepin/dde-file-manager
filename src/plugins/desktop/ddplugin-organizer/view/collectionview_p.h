/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef COLLECTIONVIEW_P_H
#define COLLECTIONVIEW_P_H

#include "view/collectionview.h"

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionViewPrivate
{
public:
    explicit CollectionViewPrivate(CollectionView *qq);

    void updateViewSizeData(const QSize &viewSize, const QMargins &viewMargins, const QSize &itemSize);

    int verticalScrollToValue(const QModelIndex &index, const QRect &rect, QAbstractItemView::ScrollHint hint) const;
    QItemSelection selection(const QRect &rect) const;

    QPoint pointToPos(const QPoint &point) const;
    QPoint posToPoint(const QPoint &pos) const;
    int posToNode(const QPoint &pos) const;
    QPoint nodeToPos(const int node) const;

private:
    void updateRowCount(const int &viewHeight, const int &minCellHeight);
    void updateColumnCount(const int &viewWidth, const int &minCellWidth);
    void updateCellMargins(const QSize &itemSize, const QSize &cellSize);
    void updateViewMargins(const QSize &viewSize, const QMargins &oldMargins);

public:
    int space = 0;
    QMargins viewMargins;
    QMargins cellMargins = QMargins(2, 2, 2, 2);
    int rowCount = 1;
    int columnCount = 1;
    int cellWidth = 1;
    int cellHeight = 1;

    QList<QUrl> urls;

    bool showGrid = false;

private:
    CollectionView *q;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONVIEW_P_H
