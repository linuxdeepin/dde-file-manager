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
