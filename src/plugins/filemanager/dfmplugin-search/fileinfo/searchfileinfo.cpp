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

namespace dfmplugin_search {

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
}

SearchFileInfo::~SearchFileInfo()
{
}

bool SearchFileInfo::exists() const
{
    if (SearchHelper::isRootUrl(url()))
        return true;

    return AbstractFileInfo::exists();
}

bool SearchFileInfo::isHidden() const
{
    if (SearchHelper::isRootUrl(url()))
        return false;

    return AbstractFileInfo::isHidden();
}

bool SearchFileInfo::isReadable() const
{
    if (SearchHelper::isRootUrl(url()))
        return true;

    return AbstractFileInfo::isReadable();
}

bool SearchFileInfo::isWritable() const
{
    if (SearchHelper::isRootUrl(url()))
        return true;

    return AbstractFileInfo::isWritable();
}

bool SearchFileInfo::isDir() const
{
    if (SearchHelper::isRootUrl(url()))
        return true;

    return AbstractFileInfo::isDir();
}

qint64 SearchFileInfo::size() const
{
    if (SearchHelper::isRootUrl(url()))
        return -1;

    return AbstractFileInfo::size();
}

QString SearchFileInfo::emptyDirectoryTip() const
{
    return QObject::tr("No results");
}

QString SearchFileInfo::loadingTip() const
{
    return QObject::tr("Searching...");
}

}
