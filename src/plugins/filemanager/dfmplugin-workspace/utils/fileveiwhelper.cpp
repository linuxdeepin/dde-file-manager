/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "fileveiwhelper.h"
#include "views/fileview.h"

DPWORKSPACE_USE_NAMESPACE

bool FileVeiwHelper::isEmptyArea(const FileView *view, const QPoint &pos)
{
    const QModelIndex &index = view->indexAt(pos);

    return !index.isValid();
}

int FileVeiwHelper::caculateListItemIndex(const QSize &itemSize, const QPoint &pos)
{
    if (pos.y() % (itemSize.height() + kListViewSpacing * 2) < kListViewSpacing)
        return -1;

    int itemHeight = itemSize.height() + kListViewSpacing * 2;
    return pos.y() / itemHeight;
}

int FileVeiwHelper::caculateIconItemIndex(const FileView *view, const QSize &itemSize, const QPoint &pos)
{
    int itemHeight = itemSize.height() + kIconViewSpacing * 2;

    if (pos.y() % itemHeight < (kIconViewSpacing + kIconModeColumnPadding)
        || pos.y() % itemHeight > (itemHeight - kIconModeColumnPadding))
        return -1;

    int itemWidth = itemSize.width() + kIconViewSpacing * 2;

    if (pos.x() % itemWidth <= (kIconViewSpacing + kIconModeColumnPadding)
        || pos.x() % itemWidth > (itemHeight - kIconModeColumnPadding))
        return -1;

    int columnIndex = pos.x() / itemWidth;
    int contentWidth = view->maximumViewportSize().width();
    int columnCount = qMax((contentWidth - 1) / itemWidth, 1);

    if (columnIndex >= columnCount)
        return -1;

    int rowIndex = pos.y() / (itemSize.height() + kIconViewSpacing * 2);
    return rowIndex * columnCount + columnIndex;
}
