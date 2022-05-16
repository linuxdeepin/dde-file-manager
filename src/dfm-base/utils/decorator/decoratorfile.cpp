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

#include "decoratorfile.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

DFMBASE_BEGIN_NAMESPACE

class DecoratorFilePrivate
{
public:
    explicit DecoratorFilePrivate(DecoratorFile *q)
        : q(q)
    {
    }
    ~DecoratorFilePrivate() = default;

public:
    DecoratorFile *q = nullptr;
    QSharedPointer<DFMIO::DFile> dfile = nullptr;
};

DFMBASE_END_NAMESPACE

DFMBASE_USE_NAMESPACE

DecoratorFile::DecoratorFile(const QString &filePath)
    : d(new DecoratorFilePrivate(this))
{
    const QUrl &url = QUrl::fromLocalFile(filePath);
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (factory)
        d->dfile = factory->createFile();
}

DecoratorFile::DecoratorFile(const QUrl &url)
    : d(new DecoratorFilePrivate(this))
{
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (factory)
        d->dfile = factory->createFile();
}

DecoratorFile::DecoratorFile(QSharedPointer<dfmio::DFile> dfile)
    : d(new DecoratorFilePrivate(this))
{
    d->dfile = dfile;
}

QSharedPointer<dfmio::DFile> DecoratorFile::filePtr()
{
    return d->dfile;
}

bool DecoratorFile::exists() const
{
    if (d->dfile)
        return d->dfile->exists();
    return false;
}

qint64 DecoratorFile::size() const
{
    if (d->dfile)
        return d->dfile->size();
    return -1;
}

QByteArray DecoratorFile::readAll() const
{
    if (d->dfile) {
        if (!d->dfile->isOpen())
            d->dfile->open(DFMIO::DFile::OpenFlag::kReadOnly);
        QByteArray ret = d->dfile->readAll();
        d->dfile->close();

        return ret;
    }
    return QByteArray();
}

qint64 DecoratorFile::writeAll(const QByteArray &byteArray)
{
    if (d->dfile) {
        if (!d->dfile->isOpen()) {
            bool ok = d->dfile->open(DFMIO::DFile::OpenFlag::kWriteOnly);
            if (!ok)
                return -1;
        }
        qint64 ret = d->dfile->write(byteArray);
        d->dfile->close();

        return ret;
    }
    return -1;
}

DFMIO::DFile::Permissions DecoratorFile::permissions() const
{
    if (d->dfile)
        return d->dfile->permissions();
    return DFMIO::DFile::Permission::kNoPermission;
}

bool DecoratorFile::setPermissions(DFMIO::DFile::Permissions permission)
{
    if (d->dfile)
        return d->dfile->setPermissions(permission);
    return false;
}

DFMIOError DecoratorFile::lastError() const
{
    if (d->dfile)
        return d->dfile->lastError();
    return DFMIOError();
}
