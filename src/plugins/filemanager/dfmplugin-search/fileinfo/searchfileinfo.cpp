// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchfileinfo.h"
#include "utils/searchhelper.h"

#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/base/schemefactory.h>

namespace dfmplugin_search {

SearchFileInfo::SearchFileInfo(const QUrl &url)
    : FileInfo(url)
{
}

SearchFileInfo::~SearchFileInfo()
{
}

bool SearchFileInfo::exists() const
{
    if (SearchHelper::isRootUrl(url))
        return true;

    return FileInfo::exists();
}

Qt::DropActions SearchFileInfo::supportedOfAttributes(const SupportType type) const
{
    if (SearchHelper::isRootUrl(url) && type == SupportType::kDrop)
        return Qt::IgnoreAction;

    return FileInfo::supportedOfAttributes(type);
}

bool SearchFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsDir:
        if (SearchHelper::isRootUrl(url))
            return true;
        return FileInfo::isAttributes(type);
    case FileIsType::kIsReadable:
        if (SearchHelper::isRootUrl(url))
            return true;
        return FileInfo::isAttributes(type);
    case FileIsType::kIsWritable:
        if (SearchHelper::isRootUrl(url))
            return true;

        return FileInfo::isAttributes(type);
    case FileIsType::kIsHidden:
        if (SearchHelper::isRootUrl(url))
            return false;

        return FileInfo::isAttributes(type);
    default:
        return FileInfo::isAttributes(type);
    }
}

qint64 SearchFileInfo::size() const
{
    if (SearchHelper::isRootUrl(url))
        return -1;

    return FileInfo::size();
}

QString SearchFileInfo::displayOf(const FileInfo::DisplayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (UrlRoute::isRootUrl(url)) {
            return QObject::tr("Search");
        }
    }

    return FileInfo::displayOf(type);
}

QString SearchFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        if (SearchHelper::isRootUrl(url))
            return QObject::tr("Search");
        [[fallthrough]];
    default:
        return FileInfo::nameOf(type);
    }
}

QString SearchFileInfo::viewOfTip(const ViewInfoType type) const
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

}
