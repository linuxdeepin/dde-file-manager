// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shareiterator.h"
#include "private/shareiterator_p.h"
#include "utils/shareutils.h"
#include "fileinfo/sharefileinfo.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/file/local/localdiriterator.h>

#include <dfm-framework/event/event.h>

using namespace dfmplugin_myshares;
DFMBASE_USE_NAMESPACE

ShareIterator::ShareIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new ShareIteratorPrivate(this, url))
{
    if (!UniversalUtils::urlEquals(url, ShareUtils::rootUrl()))
        d->proxy = new LocalDirIterator(ShareUtils::convertToLocalUrl(url), nameFilters, filters, flags);
}

ShareIterator::~ShareIterator()
{
}

QUrl ShareIterator::next()
{
    if (d->proxy)
        return ShareUtils::makeShareUrl(d->proxy->next().path());

    if (d->shares.isEmpty())
        return {};

    d->currentInfo = d->shares.takeFirst();

    return fileUrl();
}

bool ShareIterator::hasNext() const
{
    if (d->proxy)
        return d->proxy->hasNext();

    return !d->shares.isEmpty();
}

QString ShareIterator::fileName() const
{
    if (d->proxy)
        return d->proxy->fileName();

    return d->currentInfo.value(ShareInfoKeys::kName).toString();
}

QUrl ShareIterator::fileUrl() const
{
    if (d->proxy)
        return ShareUtils::makeShareUrl(d->proxy->fileUrl().path());

    return ShareUtils::makeShareUrl(d->currentInfo.value(ShareInfoKeys::kPath).toString());
}

const FileInfoPointer ShareIterator::fileInfo() const
{
    return InfoFactory::create<FileInfo>(fileUrl());
}

QUrl ShareIterator::url() const
{
    if (d->rootUrl.isValid())
        return d->rootUrl;

    return ShareUtils::rootUrl();
}

ShareIteratorPrivate::ShareIteratorPrivate(ShareIterator *qq, const QUrl &url)
    : q(qq)
{
    shares = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_AllShareInfos").value<QList<QVariantMap>>();
    rootUrl = url;
}

ShareIteratorPrivate::~ShareIteratorPrivate()
{
}
