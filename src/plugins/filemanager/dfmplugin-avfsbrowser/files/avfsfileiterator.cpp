// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsfileiterator.h"
#include "private/avfsfileiterator_p.h"
#include "utils/avfsutils.h"

#include <QDebug>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE

AvfsFileIterator::AvfsFileIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(AvfsUtils::avfsUrlToLocal(url), nameFilters, filters, flags), d(new AvfsFileIteratorPrivate(url, this))
{
    d->proxy = new LocalDirIterator(AvfsUtils::avfsUrlToLocal(url), nameFilters, filters, flags);
}

AvfsFileIterator::~AvfsFileIterator()
{
}

AvfsFileIteratorPrivate::AvfsFileIteratorPrivate(const QUrl &root, AvfsFileIterator *qq)
    : q(qq), root(root)
{
}

AvfsFileIteratorPrivate::~AvfsFileIteratorPrivate()
{
    if (proxy)
        delete proxy;
}

QUrl AvfsFileIterator::next()
{
    return AvfsUtils::localUrlToAvfsUrl(d->proxy->next());
}

bool AvfsFileIterator::hasNext() const
{
    return d->proxy->hasNext();
}

QString AvfsFileIterator::fileName() const
{
    return d->proxy->fileName();
}

QUrl AvfsFileIterator::fileUrl() const
{
    return AvfsUtils::localUrlToAvfsUrl(d->proxy->fileUrl());
}

const FileInfoPointer AvfsFileIterator::fileInfo() const
{
    return nullptr;
}

QUrl AvfsFileIterator::url() const
{
    return d->root;
}
