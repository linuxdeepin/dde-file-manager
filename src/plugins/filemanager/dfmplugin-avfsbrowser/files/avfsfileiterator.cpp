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

DPAVFSBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AvfsFileIterator::AvfsFileIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : LocalDirIterator(url, nameFilters, filters, flags), d(new AvfsFileIteratorPrivate(this))
{
}

AvfsFileIterator::~AvfsFileIterator()
{
}

AvfsFileIteratorPrivate::AvfsFileIteratorPrivate(AvfsFileIterator *qq)
    : q(qq)
{
}

QUrl AvfsFileIterator::next()
{
    return {};
}

bool AvfsFileIterator::hasNext() const
{
    return false;
}

QString AvfsFileIterator::fileName() const
{
    return "";
}

QUrl AvfsFileIterator::fileUrl() const
{
    return {};
}

const AbstractFileInfoPointer AvfsFileIterator::fileInfo() const
{
    return nullptr;
}

QUrl AvfsFileIterator::url() const
{
    return {};
}
