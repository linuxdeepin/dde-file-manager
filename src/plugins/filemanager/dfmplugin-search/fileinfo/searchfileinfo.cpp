// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchfileinfo.h"
#include "utils/searchhelper.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"

namespace dfmplugin_search {

SearchFileInfo::SearchFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
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

Qt::DropActions SearchFileInfo::supportedOfAttributes(const SupportType type) const
{
    if (SearchHelper::isRootUrl(dptr->url) && type == SupportType::kDrop)
        return Qt::IgnoreAction;

    return AbstractFileInfo::supportedOfAttributes(type);
}

bool SearchFileInfo::isAttributes(const OptInfoType type) const
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

QString SearchFileInfo::displayOf(const AbstractFileInfo::DisplayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (UrlRoute::isRootUrl(dptr->url)) {
            return QObject::tr("Search");
        }
    }

    return AbstractFileInfo::displayOf(type);
}

QString SearchFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        if (SearchHelper::isRootUrl(dptr->url))
            return QObject::tr("Search");
        [[fallthrough]];
    default:
        return AbstractFileInfo::nameOf(type);
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
