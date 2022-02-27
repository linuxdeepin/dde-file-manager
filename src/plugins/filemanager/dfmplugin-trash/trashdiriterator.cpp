/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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

#include "trashdiriterator.h"
#include "trashfileinfo.h"
#include "utils/trashhelper.h"
#include "private/trashdiriterator_p.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"

DFMBASE_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

TrashDirIteratorPrivate::TrashDirIteratorPrivate(TrashDirIterator *qq)
    : q(qq)
{
}

TrashDirIteratorPrivate::~TrashDirIteratorPrivate()
{
}

TrashDirIterator::TrashDirIterator(const QUrl &url,
                                   const QStringList &nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new TrashDirIteratorPrivate(this))
{
    QString path = StandardPaths::location(StandardPaths::kTrashFilesPath) + url.path();
    d->iterator = new QDirIterator(path, nameFilters, filters, flags);
}

TrashDirIterator::~TrashDirIterator()
{
    if (d->iterator) {
        delete d->iterator;
    }

    // Todo(yanghao):hiddenFiles
}

QUrl TrashDirIterator::next()
{
    // Todo(yanghao): cache
    return TrashHelper::fromLocalFile(d->iterator->next());
}

bool TrashDirIterator::hasNext() const
{
    // Todo(yanghao):hiddenFiles
    return d->iterator->hasNext();
}

QString TrashDirIterator::fileName() const
{
    return d->iterator->fileName();
}

QUrl TrashDirIterator::fileUrl() const
{
    return TrashHelper::fromLocalFile(d->iterator->filePath());
}

const AbstractFileInfoPointer TrashDirIterator::fileInfo() const
{
    return InfoFactory::create<AbstractFileInfo>(fileUrl());
}

QUrl TrashDirIterator::url() const
{
    return TrashHelper::rootUrl();
}
