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

#include <QDebug>

class RecentBrowseViewModelPrivate
{
    friend class RecentBrowseViewModel;
    RecentBrowseViewModel *const q;
    QList<QSharedPointer<FileViewItem>> children;
public:
    explicit RecentBrowseViewModelPrivate(RecentBrowseViewModel* qq)
        : q(qq)
    {

    }
};

RecentBrowseViewModel::RecentBrowseViewModel(QAbstractItemView *parent)
    : FileViewModel(parent)
    , d(new RecentBrowseViewModelPrivate(this))
{
    setRootUrl(RecentUtil::onlyRootUrl());
}

RecentBrowseViewModel::~RecentBrowseViewModel()
{

}

QModelIndex RecentBrowseViewModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(column);

    if(row < 0 || column < 0)
        return QModelIndex();
    return createIndex(row, column, d->children.at(row).data());
}

AbstractFileInfoPointer RecentBrowseViewModel::fileInfo(const QModelIndex &index)
{
    return FileViewModel::fileInfo(index);
}

const FileViewItem *RecentBrowseViewModel::itemFromIndex(const QModelIndex &index) const
{
    if (0 > index.row() || index.row() >= d->children.size())
        return nullptr;
    return d->children.at(index.row()).data();
}

QModelIndex RecentBrowseViewModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int RecentBrowseViewModel::rowCount(const QModelIndex &parent) const
{
    return d->children.count();
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
    FileViewModel::fetchMore(parent);
    beginResetModel();
    RecentUtil::initRecentSubSystem();
    for (int i = 0 ; i < RecentUtil::getRecentNodes().size(); i++) {
        QUrl url = QUrl(RecentUtil::getRecentNodes().at(i).toElement().attribute("href"));
        QUrl schemeUrl = UrlRoute::pathToUrl(url.path());
        if (!schemeUrl.isValid())
            continue;
        d->children.append(QSharedPointer<FileViewItem>(new FileViewItem(schemeUrl)));
    }
    endResetModel();
}

bool RecentBrowseViewModel::canFetchMore(const QModelIndex &parent) const
{
    return FileViewModel::canFetchMore(parent);
}
