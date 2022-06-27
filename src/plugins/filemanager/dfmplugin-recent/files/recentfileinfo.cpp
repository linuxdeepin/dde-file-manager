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

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
namespace dfmplugin_recent {

class RecentFileInfoPrivate : public AbstractFileInfoPrivate
{
public:
    explicit RecentFileInfoPrivate(AbstractFileInfo *qq)
        : AbstractFileInfoPrivate(qq)
    {
    }

    virtual ~RecentFileInfoPrivate();
};

RecentFileInfoPrivate::~RecentFileInfoPrivate()
{
}

RecentFileInfo::RecentFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new RecentFileInfoPrivate(this))
{
    d = static_cast<RecentFileInfoPrivate *>(dptr.data());
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

QString RecentFileInfo::fileName() const
{
    if (d->proxy)
        return d->proxy->fileName();

    if (UrlRoute::isRootUrl(url()))
        return QObject::tr("Recent");

    return QString();
}

bool RecentFileInfo::canRedirectionFileUrl() const
{
    return d->proxy;
}

QUrl RecentFileInfo::redirectedFileUrl() const
{
    return d->proxy ? d->proxy->url() : url();
};

}
