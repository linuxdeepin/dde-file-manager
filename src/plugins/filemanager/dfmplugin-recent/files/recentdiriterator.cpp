// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentdiriterator.h"
#include "files/recentfileinfo.h"
#include "utils/recentmanager.h"
#include "private/recentdiriterator_p.h"

#include <dfm-base/base/schemefactory.h>

using namespace dfmplugin_recent;
DFMBASE_USE_NAMESPACE

RecentDirIteratorPrivate::RecentDirIteratorPrivate(RecentDirIterator *qq)
    : q(qq)
{
    recentNodes = RecentManager::instance()->getRecentNodes();
    for (QUrl url : recentNodes.keys())
        urlList << url;
}

RecentDirIteratorPrivate::~RecentDirIteratorPrivate()
{
}

RecentDirIterator::RecentDirIterator(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new RecentDirIteratorPrivate(this))
{
}

RecentDirIterator::~RecentDirIterator()
{
}

QUrl RecentDirIterator::next()
{
    if (!d->urlList.isEmpty()) {
        d->currentUrl = d->urlList.dequeue();
        return d->currentUrl;
    }
    return QUrl();
}

bool RecentDirIterator::hasNext() const
{
    if (!d->urlList.isEmpty()) {
        return true;
    }

    return false;
}

QString RecentDirIterator::fileName() const
{
    FileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->nameOf(NameInfoType::kFileName) : QString();
}

QUrl RecentDirIterator::fileUrl() const
{
    FileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->urlOf(UrlInfoType::kUrl) : QString();
}

const FileInfoPointer RecentDirIterator::fileInfo() const
{
    return d->recentNodes.value(d->currentUrl);
}

QUrl RecentDirIterator::url() const
{
    return RecentHelper::rootUrl();
}
