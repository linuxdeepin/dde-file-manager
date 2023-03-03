// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsfileiterator.h"
#include "private/avfsfileiterator_p.h"
#include "utils/avfsutils.h"

#include <QDebug>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE

AvfsFileIterator::AvfsFileIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : LocalDirIterator(AvfsUtils::avfsUrlToLocal(url), nameFilters, filters, flags), d(new AvfsFileIteratorPrivate(url, this))
{
}

AvfsFileIterator::~AvfsFileIterator()
{
}

AvfsFileIteratorPrivate::AvfsFileIteratorPrivate(const QUrl &root, AvfsFileIterator *qq)
    : q(qq), root(root)
{
}

QUrl AvfsFileIterator::next()
{
    return AvfsUtils::localUrlToAvfsUrl(LocalDirIterator::next());
}

bool AvfsFileIterator::hasNext() const
{
    return LocalDirIterator::hasNext();
}

QString AvfsFileIterator::fileName() const
{
    return LocalDirIterator::fileName();
}

QUrl AvfsFileIterator::fileUrl() const
{
    return AvfsUtils::localUrlToAvfsUrl(LocalDirIterator::fileUrl());
}

const AbstractFileInfoPointer AvfsFileIterator::fileInfo() const
{
    return nullptr;
}

QUrl AvfsFileIterator::url() const
{
    return d->root;
}
