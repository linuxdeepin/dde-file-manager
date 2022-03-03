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

SearchFileInfo::SearchFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new SearchFileInfoPrivate(this))
{
    d = static_cast<SearchFileInfoPrivate *>(dptr.data());
    if (!SearchHelper::isRootUrl(url))
        setProxy(InfoFactory::create<AbstractFileInfo>(SearchHelper::searchedFileUrl(url)));
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

QString SearchFileInfo::filePath() const
{
    if (d->proxy)
        return d->proxy->filePath();

    return {};
}

QIcon SearchFileInfo::fileIcon() const
{
    if (d->proxy)
        return d->proxy->fileIcon();

    return {};
}

bool SearchFileInfo::exists() const
{
    if (!d->proxy)
        return true;

    return d->proxy->exists();
}

bool SearchFileInfo::isHidden() const
{
    if (!d->proxy)
        return false;

    return d->proxy->isHidden();
}

bool SearchFileInfo::isReadable() const
{
    if (!d->proxy)
        return true;

    return d->proxy->isReadable();
}

bool SearchFileInfo::isWritable() const
{
    if (!d->proxy)
        return true;

    return d->proxy->isWritable();
}

bool SearchFileInfo::isDir() const
{
    if (!d->proxy)
        return true;

    return d->proxy->isDir();
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

QDateTime SearchFileInfo::lastRead() const
{
    if (d->proxy)
        return d->proxy->lastRead();

    return {};
}

QDateTime SearchFileInfo::created() const
{
    if (d->proxy)
        return d->proxy->created();

    return {};
}

QString SearchFileInfo::fileTypeDisplayName() const
{
    if (d->proxy)
        return d->proxy->fileTypeDisplayName();

    return {};
}

QString SearchFileInfo::emptyDirectoryTip() const
{
    return QObject::tr("No results");
}

QString SearchFileInfo::loadingTip() const
{
    return QObject::tr("Searching...");
}

DPSEARCH_END_NAMESPACE
