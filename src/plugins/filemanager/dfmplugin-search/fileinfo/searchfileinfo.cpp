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
#include "utils/searchhelper.h"
#include "searchfileinfo_p.h"

#include "dfm-base/base/schemefactory.h"

namespace dfmplugin_search {

SearchFileInfo::SearchFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
    dptr.reset(new SearchFileInfoPrivate(url, this));
}

SearchFileInfo::~SearchFileInfo()
{
}

bool SearchFileInfo::exists() const
{
    if (SearchHelper::isRootUrl(dptr->url))
        return true;

    return AbstractFileInfo::exists();
}

bool SearchFileInfo::isAttributes(const AbstractFileInfo::FileIsType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        if (SearchHelper::isRootUrl(dptr->url))
            return true;
        return AbstractFileInfo::isAttributes(type);
    case FileIsType::kIsReadable:
        if (SearchHelper::isRootUrl(dptr->url))
            return true;
        return AbstractFileInfo::isAttributes(type);
    case FileIsType::kIsWritable:
        if (SearchHelper::isRootUrl(dptr->url))
            return true;

        return AbstractFileInfo::isAttributes(type);
    case FileIsType::kIsHidden:
        if (SearchHelper::isRootUrl(dptr->url))
            return false;

        return AbstractFileInfo::isAttributes(type);
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

qint64 SearchFileInfo::size() const
{
    if (SearchHelper::isRootUrl(dptr->url))
        return -1;

    return AbstractFileInfo::size();
}

QString SearchFileInfo::nameInfo(const AbstractFileInfo::FileNameInfoType type) const
{
    switch (type) {
    case AbstractFileInfo::FileNameInfoType::kFileName:
        if (SearchHelper::isRootUrl(dptr->url))
            return QObject::tr("Search");
        [[fallthrough]];
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

QString SearchFileInfo::viewTip(const AbstractFileInfo::ViewType type) const
{
    switch (type) {
    case ViewType::kEmptyDir:
        return QObject::tr("No results");
    case ViewType::kLoading:
        return QObject::tr("Searching...");
    default:
        return QString();
    }
}

SearchFileInfoPrivate::SearchFileInfoPrivate(const QUrl &url, SearchFileInfo *qq)
    : AbstractFileInfoPrivate(url, qq)
{
}

SearchFileInfoPrivate::~SearchFileInfoPrivate()
{
}

}
