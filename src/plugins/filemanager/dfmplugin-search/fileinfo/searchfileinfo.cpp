/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "searchfileinfo.h"
#include "searchfileinfo_p.h"
#include "utils/searchhelper.h"

#include "dfm-base/base/schemefactory.h"

DPSEARCH_BEGIN_NAMESPACE

SearchFileInfoPrivate::SearchFileInfoPrivate(AbstractFileInfo *qq)
    : AbstractFileInfoPrivate(qq)
{
}

SearchFileInfoPrivate::~SearchFileInfoPrivate()
{
}

void SearchFileInfoPrivate::setProxy(const AbstractFileInfoPointer &aproxy)
{
    proxy = aproxy;
}

SearchFileInfo::SearchFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new SearchFileInfoPrivate(this))
{
    d = static_cast<SearchFileInfoPrivate *>(dptr.data());
    if (SearchHelper::searchedFileUrl(url).isValid())
        d->setProxy(InfoFactory::create<AbstractFileInfo>(SearchHelper::searchedFileUrl(url)));
}

SearchFileInfo::~SearchFileInfo()
{
}

QString SearchFileInfo::fileName() const
{
    if (d->proxy)
        return d->proxy->fileName();

    return {};
}

bool SearchFileInfo::exists() const
{
    if (url() == SearchHelper::rootUrl())
        return true;

    return d->proxy && d->proxy->exists();
}

bool SearchFileInfo::isHidden() const
{
    return d->proxy && d->proxy->isHidden();
}

qint64 SearchFileInfo::size() const
{
    if (d->proxy)
        return d->proxy->size();

    return -1;
}

QString SearchFileInfo::sizeFormat() const
{
    if (d->proxy)
        return d->proxy->sizeFormat();

    return {};
}

QDateTime SearchFileInfo::lastModified() const
{
    if (d->proxy)
        return d->proxy->lastModified();

    return {};
}

DPSEARCH_END_NAMESPACE
