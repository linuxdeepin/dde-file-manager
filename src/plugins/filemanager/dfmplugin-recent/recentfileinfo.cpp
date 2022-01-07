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
        d->setProxy(InfoFactory::create<AbstractFileInfo>(url));
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

DPRECENT_END_NAMESPACE
