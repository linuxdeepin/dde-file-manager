/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "recentfileinfo.h"
#include "utils/recentmanager.h"

#include "dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

DFMBASE_USE_NAMESPACE
namespace dfmplugin_recent {

RecentFileInfo::RecentFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
    if (url.path() != "/") {
        setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(url.path())));
    }
}

RecentFileInfo::~RecentFileInfo()
{
}

bool RecentFileInfo::exists() const
{
    return AbstractFileInfo::exists() || dptr->url == RecentManager::rootUrl();
}

QFile::Permissions RecentFileInfo::permissions() const
{
    if (dptr->url == RecentManager::rootUrl()) {
        return QFileDevice::ReadGroup | QFileDevice::ReadOwner | QFileDevice::ReadOther;
    }
    return AbstractFileInfo::permissions();
}

bool RecentFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsReadable:
        return permissions().testFlag(QFile::Permission::ReadUser);
    case FileIsType::kIsWritable:
        return permissions().testFlag(QFile::Permission::WriteUser);
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

bool RecentFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanRename:
        return false;
    case FileCanType::kCanRedirectionFileUrl:
        return dptr->proxy;
    default:
        return AbstractFileInfo::canAttributes(type);
    }
}

QString RecentFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case NameInfoType::kFileName:
        if (dptr->proxy)
            return dptr->proxy->nameOf(NameInfoType::kFileName);

        if (UrlRoute::isRootUrl(dptr->url))
            return QObject::tr("Recent");

        return QString();
    default:
        return AbstractFileInfo::nameOf(type);
    }
}

QUrl RecentFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return dptr->proxy ? dptr->proxy->urlOf(UrlInfoType::kUrl) : dptr->url;
    default:
        return AbstractFileInfo::urlOf(type);
    }
}

QVariant RecentFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFilePathRole)
        return urlOf(UrlInfoType::kRedirectedFileUrl).path();
    else if (role == kItemFileLastReadRole)
        return timeOf(TimeInfoType::kLastRead).value<QDateTime>().toString(FileUtils::dateTimeFormat());
    else
        return QVariant();
}

QString RecentFileInfo::displayOf(const AbstractFileInfo::DisplayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (UrlRoute::isRootUrl(dptr->url)) {
            return QObject::tr("Recent");
        }
    }
    return AbstractFileInfo::displayOf(type);
}

}
