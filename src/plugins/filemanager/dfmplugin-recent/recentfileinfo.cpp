/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "recentfileinfo.h"
#include "utils/recentmanager.h"

#include "interfaces/private/abstractfileinfo_p.h"
#include "dfm-base/base/schemefactory.h"

DPRECENT_USE_NAMESPACE

DPRECENT_BEGIN_NAMESPACE

class RecentFileInfoPrivate : public AbstractFileInfoPrivate
{
public:
    explicit RecentFileInfoPrivate(AbstractFileInfo *qq)
        : AbstractFileInfoPrivate(qq)
    {
    }

    virtual ~RecentFileInfoPrivate();
    void setProxy(const AbstractFileInfoPointer &proxy);

public:
    AbstractFileInfoPointer proxy;
};

RecentFileInfoPrivate::~RecentFileInfoPrivate()
{
}

void RecentFileInfoPrivate::setProxy(const AbstractFileInfoPointer &aproxy)
{
    proxy = aproxy;
}

RecentFileInfo::RecentFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new RecentFileInfoPrivate(this))
{
    d = static_cast<RecentFileInfoPrivate *>(dptr.data());
    if (url.path() != "/") {
        d->setProxy(InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(url.path())));
    }
}

RecentFileInfo::~RecentFileInfo()
{
}

QString RecentFileInfo::fileName() const
{
    if (d->proxy) {
        return d->proxy->fileName();
    }
    return QString();
}

bool RecentFileInfo::exists() const
{
    if (url() == RecentManager::rootUrl())
        return true;
    return d->proxy && d->proxy->exists();
}

bool RecentFileInfo::isHidden() const
{
    return d->proxy && d->proxy->isHidden();
}

qint64 RecentFileInfo::size() const
{
    if (d->proxy) {
        return d->proxy->size();
    }
    return -1;
}

QString RecentFileInfo::sizeFormat() const
{
    if (d->proxy) {
        return d->proxy->sizeFormat();
    }
    return QString();
}

QDateTime RecentFileInfo::lastModified() const
{
    if (d->proxy) {
        return d->proxy->lastModified();
    }
    return QDateTime();
}

QFile::Permissions RecentFileInfo::permissions() const
{
    if (url() == RecentManager::rootUrl()) {
        return QFileDevice::ReadGroup | QFileDevice::ReadOwner | QFileDevice::ReadOther;
    }
    if (d->proxy)
        return d->proxy->permissions();

    return QFile::Permissions();
}

bool RecentFileInfo::isReadable() const
{
    return permissions().testFlag(QFile::Permission::ReadUser);
}

bool RecentFileInfo::isWritable() const
{
    // Todo(yanghao): gvfs优化
    return permissions().testFlag(QFile::Permission::WriteUser);
};

DPRECENT_END_NAMESPACE
