// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagdiriterator.h"
#include "private/tagdiriterator_p.h"

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

TagDirIterator::TagDirIterator(const QUrl &url,
                               const QStringList &nameFilters,
                               QDir::Filters filters,
                               QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new TagDirIteratorPrivate(this))
{
    d->rootUrl = url;
    d->loadTagsUrls(url);
}

TagDirIterator::~TagDirIterator()
{
}

QUrl TagDirIterator::next()
{
    if (!d->urlList.isEmpty()) {
        d->currentUrl = d->urlList.dequeue();
        return d->currentUrl;
    }

    return QUrl();
}

bool TagDirIterator::hasNext() const
{
    if (!d->urlList.isEmpty())
        return true;

    return false;
}

QString TagDirIterator::fileName() const
{
    FileInfoPointer currentInfo = d->tagNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->nameOf(NameInfoType::kFileName) : QString();
}

QUrl TagDirIterator::fileUrl() const
{
    FileInfoPointer currentInfo = d->tagNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->urlOf(UrlInfoType::kUrl) : QString();
}

const FileInfoPointer TagDirIterator::fileInfo() const
{
    return d->tagNodes.value(d->currentUrl);
}

QUrl TagDirIterator::url() const
{
    return d->rootUrl;
}
