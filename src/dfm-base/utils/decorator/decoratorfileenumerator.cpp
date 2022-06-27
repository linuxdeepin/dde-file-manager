/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#include "decoratorfileenumerator.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QRegularExpression>

namespace dfmbase {

class DecoratorFileEnumeratorPrivate
{
public:
    explicit DecoratorFileEnumeratorPrivate(DecoratorFileEnumerator *q)
        : q(q)
    {
    }
    ~DecoratorFileEnumeratorPrivate() = default;

public:
    DecoratorFileEnumerator *q = nullptr;
    QSharedPointer<DFMIO::DEnumerator> denumerator = nullptr;
    QUrl url;
};

}

using namespace dfmbase;

static constexpr char kNetworkFilesRex[] { R"((^/run/user/.*/gvfs/|^/root/.gvfs/)(sftp|ftp|smb|dav))" };

DecoratorFileEnumerator::DecoratorFileEnumerator(const QString &filePath,
                                                 const QStringList &nameFilters /*= QStringList()*/,
                                                 DFMIO::DEnumerator::DirFilters filters /*= DFMIO::DEnumerator::DirFilter::NoFilter*/,
                                                 DFMIO::DEnumerator::IteratorFlags flags /*= DFMIO::DEnumerator::IteratorFlag::NoIteratorFlags*/)
    : d(new DecoratorFileEnumeratorPrivate(this))
{
    d->url = QUrl::fromLocalFile(filePath);
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(d->url.scheme(), static_cast<QUrl>(d->url));
    if (factory) {
        d->denumerator = factory->createEnumerator(nameFilters, filters, flags);
        if (filePath.contains(QRegularExpression(kNetworkFilesRex)))
            d->denumerator->setTimeout(2000);
    }
}

DecoratorFileEnumerator::DecoratorFileEnumerator(const QUrl &url,
                                                 const QStringList &nameFilters /*= QStringList()*/,
                                                 DFMIO::DEnumerator::DirFilters filters /*= DFMIO::DEnumerator::DirFilter::NoFilter*/,
                                                 DFMIO::DEnumerator::IteratorFlags flags /*= DFMIO::DEnumerator::IteratorFlag::NoIteratorFlags*/)
    : d(new DecoratorFileEnumeratorPrivate(this))
{
    d->url = url;
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (factory) {
        d->denumerator = factory->createEnumerator(nameFilters, filters, flags);
        if (url.path().contains(QRegularExpression(kNetworkFilesRex)))
            d->denumerator->setTimeout(2000);
    }
}

DecoratorFileEnumerator::DecoratorFileEnumerator(QSharedPointer<dfmio::DEnumerator> dfileEnumerator)
    : d(new DecoratorFileEnumeratorPrivate(this))
{
    d->url = dfileEnumerator->uri();
    d->denumerator = dfileEnumerator;
}

QSharedPointer<dfmio::DEnumerator> DecoratorFileEnumerator::enumeratorPtr()
{
    return d->denumerator;
}

bool DecoratorFileEnumerator::isValid() const
{
    return d->denumerator;
}

bool DecoratorFileEnumerator::hasNext() const
{
    if (d->denumerator)
        return d->denumerator->hasNext();

    return false;
}

QUrl DecoratorFileEnumerator::next() const
{
    if (d->denumerator)
        return d->denumerator->next();

    return QUrl();
}

QUrl DecoratorFileEnumerator::nextUrl() const
{
    return next();
}

quint64 DecoratorFileEnumerator::fileCount() const
{
    if (d->denumerator)
        return d->denumerator->fileCount();
    return 0;
}

QSharedPointer<dfmio::DFileInfo> DecoratorFileEnumerator::fileInfo() const
{
    if (d->denumerator)
        return d->denumerator->fileInfo();

    return nullptr;
}

DFMIOError DecoratorFileEnumerator::lastError() const
{
    if (d->denumerator)
        return d->denumerator->lastError();
    return DFMIOError();
}
