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
#include "private/recentfileinfo_p.h"

#include "dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

DFMBASE_USE_NAMESPACE
namespace dfmplugin_recent {
RecentFileInfoPrivate::RecentFileInfoPrivate(const QUrl &url, RecentFileInfo *qq)
    : dfmbase::AbstractFileInfoPrivate(url, qq)
{
}

RecentFileInfoPrivate::~RecentFileInfoPrivate()
{
}

RecentFileInfo::RecentFileInfo(const QUrl &url)
    : AbstractFileInfo(url), d(new RecentFileInfoPrivate(url, this))
{
    dptr.reset(d);
    if (url.path() != "/") {
        setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(url.path())));
    }
}

RecentFileInfo::~RecentFileInfo()
{
}

bool RecentFileInfo::exists() const
{
    return AbstractFileInfo::exists() || url() == RecentManager::rootUrl();
}

QFile::Permissions RecentFileInfo::permissions() const
{
    if (url() == RecentManager::rootUrl()) {
        return QFileDevice::ReadGroup | QFileDevice::ReadOwner | QFileDevice::ReadOther;
    }
    return AbstractFileInfo::permissions();
}

bool RecentFileInfo::isReadable() const
{
    return permissions().testFlag(QFile::Permission::ReadUser);
}

bool RecentFileInfo::isWritable() const
{
    // Todo(yanghao): gvfs优化
    return permissions().testFlag(QFile::Permission::WriteUser);
}

bool RecentFileInfo::canRename() const
{
    return false;
}

QString RecentFileInfo::nameInfo(const AbstractFileInfo::FileNameInfoType type) const
{
    switch (type) {
    case AbstractFileInfo::FileNameInfoType::kFileName:
        if (dptr->proxy)
            return dptr->proxy->nameInfo(AbstractFileInfo::FileNameInfoType::kFileName);

        if (UrlRoute::isRootUrl(url()))
            return QObject::tr("Recent");

        return QString();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}

bool RecentFileInfo::canRedirectionFileUrl() const
{
    return dptr->proxy;
}

QUrl RecentFileInfo::redirectedFileUrl() const
{
    return dptr->proxy ? dptr->proxy->url() : url();
};

QVariant RecentFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFilePathRole)
        return redirectedFileUrl().path();
    else if (role == kItemFileLastReadRole)
        return lastRead().toString(FileUtils::dateTimeFormat());
    else
        return QVariant();
}

}
