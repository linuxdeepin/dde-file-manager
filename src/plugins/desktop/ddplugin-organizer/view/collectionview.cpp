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
#include "collectionview_p.h"

DDP_ORGANIZER_USE_NAMESPACE

CollectionViewPrivate::CollectionViewPrivate(CollectionView *qq) : q(qq)
{

}

CollectionView::CollectionView(QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CollectionViewPrivate(this))
{

}

QRect CollectionView::visualRect(const QModelIndex &index) const
{

}

void CollectionView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{

}

QModelIndex CollectionView::indexAt(const QPoint &point) const
{

}

QModelIndex CollectionView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{

}

int CollectionView::horizontalOffset() const
{
    return 0;
}

int CollectionView::verticalOffset() const
{

}

bool CollectionView::isIndexHidden(const QModelIndex &index) const
{

}

void CollectionView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{

}

QRegion CollectionView::visualRegionForSelection(const QItemSelection &selection) const
{

}

