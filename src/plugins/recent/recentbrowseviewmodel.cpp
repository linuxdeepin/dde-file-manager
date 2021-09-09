/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#include "recentutil.h"
#include "recentbrowseviewmodel.h"

RecentBrowseViewModel::RecentBrowseViewModel(QAbstractItemView *parent)
    : FileViewModel(parent)
{
    setRootUrl(RecentUtil::onlyRootUrl());
}

RecentBrowseViewModel::~RecentBrowseViewModel()
{

}

AbstractFileInfoPointer RecentBrowseViewModel::fileInfo(const QModelIndex &index)
{
    return FileViewModel::fileInfo(index);
}

QModelIndex RecentBrowseViewModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int RecentBrowseViewModel::rowCount(const QModelIndex &parent) const
{
    return FileViewModel::rowCount(parent);
}

int RecentBrowseViewModel::columnCount(const QModelIndex &parent) const
{
    return FileViewModel::columnCount(parent);
}

QVariant RecentBrowseViewModel::data(const QModelIndex &index, int role) const
{
    return FileViewModel::data(index,role);
}

void RecentBrowseViewModel::fetchMore(const QModelIndex &parent)
{
    return FileViewModel::fetchMore(parent);
}

bool RecentBrowseViewModel::canFetchMore(const QModelIndex &parent) const
{
    return FileViewModel::canFetchMore(parent);
}
