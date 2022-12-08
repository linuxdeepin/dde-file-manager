/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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

#include "recentdiriterator.h"
#include "files/recentfileinfo.h"
#include "files/recentiterateworker.h"
#include "utils/recentmanager.h"
#include "private/recentdiriterator_p.h"

#include "dfm-base/base/schemefactory.h"

using namespace dfmplugin_recent;

RecentDirIteratorPrivate::RecentDirIteratorPrivate(RecentDirIterator *qq)
    : q(qq)
{
    recentNodes = RecentManager::instance()->getRecentNodes();
    for (QUrl url : recentNodes.keys())
        urlList << url;
}

RecentDirIteratorPrivate::~RecentDirIteratorPrivate()
{
}

RecentDirIterator::RecentDirIterator(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new RecentDirIteratorPrivate(this))
{
}

RecentDirIterator::~RecentDirIterator()
{
}

QUrl RecentDirIterator::next()
{
    if (!d->urlList.isEmpty()) {
        d->currentUrl = d->urlList.dequeue();
        return d->currentUrl;
    }
    return QUrl();
}

bool RecentDirIterator::hasNext() const
{
    if (!d->urlList.isEmpty()) {
        return true;
    }

    return false;
}

QString RecentDirIterator::fileName() const
{
    AbstractFileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->nameInfo(NameInfo::kFileName) : QString();
}

QUrl RecentDirIterator::fileUrl() const
{
    AbstractFileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->urlInfo(dfmbase::AbstractFileInfo::FileUrlInfoType::kUrl) : QString();
}

const AbstractFileInfoPointer RecentDirIterator::fileInfo() const
{
    return d->recentNodes.value(d->currentUrl);
}

QUrl RecentDirIterator::url() const
{
    return RecentManager::rootUrl();
}
