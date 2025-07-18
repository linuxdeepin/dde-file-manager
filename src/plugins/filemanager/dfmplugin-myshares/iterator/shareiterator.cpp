// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shareiterator.h"
#include "private/shareiterator_p.h"
#include "utils/shareutils.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>

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

    return fileUrl();
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
    return QUrl::fromLocalFile(d->currentInfo.value(ShareInfoKeys::kPath).toString());
}

const FileInfoPointer ShareIterator::fileInfo() const
{
    return InfoFactory::create<FileInfo>(fileUrl());
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
