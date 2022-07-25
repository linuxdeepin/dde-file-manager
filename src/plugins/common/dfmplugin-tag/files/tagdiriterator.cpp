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
#include "tagdiriterator.h"
#include "private/tagdiriterator_p.h"

using namespace dfmplugin_tag;

TagDirIterator::TagDirIterator(const QUrl &url,
                               const QStringList &nameFilters,
                               QDir::Filters filters,
                               QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new TagDirIteratorPrivate(this))
{
    d->rootUrl = url;
    d->loadTagsUrls(url);
}

TagDirIterator::~TagDirIterator()
{
}

QUrl TagDirIterator::next()
{
    if (!d->urlList.isEmpty()) {
        d->currentUrl = d->urlList.dequeue();
        return d->currentUrl;
    }

    return QUrl();
}

bool TagDirIterator::hasNext() const
{
    if (!d->urlList.isEmpty())
        return true;

    return false;
}

QString TagDirIterator::fileName() const
{
    AbstractFileInfoPointer currentInfo = d->tagNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->fileName() : QString();
}

QUrl TagDirIterator::fileUrl() const
{
    AbstractFileInfoPointer currentInfo = d->tagNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->url() : QString();
}

const AbstractFileInfoPointer TagDirIterator::fileInfo() const
{
    return d->tagNodes.value(d->currentUrl);
}

QUrl TagDirIterator::url() const
{
    return d->rootUrl;
}
