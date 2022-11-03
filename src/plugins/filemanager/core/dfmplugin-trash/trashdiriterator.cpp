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
#include "utils/trashhelper.h"
#include "private/trashdiriterator_p.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trash;

TrashDirIteratorPrivate::TrashDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                                                 DFMIO::DEnumerator::DirFilters filters, DFMIO::DEnumerator::IteratorFlags flags,
                                                 TrashDirIterator *qq)
    : q(qq)
{
    DecoratorFileEnumerator enumerator(url, nameFilters, filters, flags);

    dEnumerator = enumerator.enumeratorPtr();
    if (!dEnumerator) {
        qWarning("Failed dfm-io use factory create enumerator");
    }
}

TrashDirIteratorPrivate::~TrashDirIteratorPrivate()
{
}

TrashDirIterator::TrashDirIterator(const QUrl &url,
                                   const QStringList &nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new TrashDirIteratorPrivate(url, nameFilters, static_cast<DFMIO::DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                    static_cast<DFMIO::DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags)), this))
{
}

TrashDirIterator::~TrashDirIterator()
{
}

QUrl TrashDirIterator::next()
{
    if (d->dEnumerator)
        d->currentUrl = d->dEnumerator->next();

    return d->currentUrl;
}

bool TrashDirIterator::hasNext() const
{
    if (d->dEnumerator)
        return d->dEnumerator->hasNext();

    return false;
}

QString TrashDirIterator::fileName() const
{
    QString path = fileUrl().path();
    if (path.isEmpty())
        return QString();

    path = path.replace(QRegExp("/*/"), "/");
    if (path == "/")
        return QString();

    if (path.endsWith("/"))
        path = path.left(path.size() - 1);
    QStringList pathList = path.split("/");
    return pathList.last();
}

QUrl TrashDirIterator::fileUrl() const
{
    return UrlRoute::pathToReal(d->currentUrl.path());
}

const AbstractFileInfoPointer TrashDirIterator::fileInfo() const
{
    return InfoFactory::create<AbstractFileInfo>(fileUrl());
}

QUrl TrashDirIterator::url() const
{
    return TrashHelper::rootUrl();
}
