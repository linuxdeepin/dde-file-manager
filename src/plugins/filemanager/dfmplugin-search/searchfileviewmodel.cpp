/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searchfileviewmodel.h"

SearchFileViewModel::SearchFileViewModel(QAbstractItemView *parent)
    : FileViewModel(parent)
{
}

SearchFileViewModel::~SearchFileViewModel()
{
}

QModelIndex SearchFileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (row < 0 || column < 0)
        return QModelIndex();

    return createIndex(row, column, m_fileNodes.at(row).data());
}

QModelIndex SearchFileViewModel::setRootUrl(const QUrl &url)
{
    const auto &index = FileViewModel::setRootUrl(url);

    // TODO 开启线程，执行搜索任务

    return index;
}

const FileViewItem *SearchFileViewModel::itemFromIndex(const QModelIndex &index) const
{
    if (0 > index.row() || index.row() >= m_fileNodes.size())
        return nullptr;

    return m_fileNodes.at(index.row()).data();
}

int SearchFileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_fileNodes.count();
}
