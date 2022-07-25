/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "shareiterator.h"
#include "private/shareiterator_p.h"
#include "utils/shareutils.h"
#include "fileinfo/sharefileinfo.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-framework/event/event.h>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

ShareIterator::ShareIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new ShareIteratorPrivate(this))
{
}

ShareIterator::~ShareIterator()
{
}

QUrl ShareIterator::next()
{
    if (d->shares.isEmpty())
        return {};
    d->currentInfo = d->shares.takeFirst();
    return ShareUtils::makeShareUrl(d->currentInfo.value(ShareInfoKeys::kPath).toString());
}

bool ShareIterator::hasNext() const
{
    return !d->shares.isEmpty();
}

QString ShareIterator::fileName() const
{
    return d->currentInfo.value(ShareInfoKeys::kName).toString();
}

QUrl ShareIterator::fileUrl() const
{
    return ShareUtils::makeShareUrl(d->currentInfo.value(ShareInfoKeys::kPath).toString());
}

const AbstractFileInfoPointer ShareIterator::fileInfo() const
{
    return InfoFactory::create<AbstractFileInfo>(fileUrl());
}

QUrl ShareIterator::url() const
{
    return ShareUtils::rootUrl();
}

ShareIteratorPrivate::ShareIteratorPrivate(ShareIterator *qq)
    : q(qq)
{
    shares = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_AllShareInfos").value<QList<QVariantMap>>();
}

ShareIteratorPrivate::~ShareIteratorPrivate()
{
}
