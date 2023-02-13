// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "decoratorfile.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

namespace dfmbase {

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

}

using namespace dfmbase;

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

qint64 DecoratorFile::writeAll(const QByteArray &byteArray, DFMIO::DFile::OpenFlag flag)
{
    if (d->dfile) {
        if (!d->dfile->isOpen()) {
            bool ok = d->dfile->open(flag);
            if (!ok)
                return -1;
        }
        qint64 ret = d->dfile->write(byteArray.data(), byteArray.length());
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
