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

#include "decoratorfileoperator.h"
#include "decoratorfileinfo.h"

#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

DFMBASE_BEGIN_NAMESPACE

class DecoratorFileOperatorPrivate
{
public:
    explicit DecoratorFileOperatorPrivate(DecoratorFileOperator *q)
        : q(q)
    {
    }
    ~DecoratorFileOperatorPrivate() = default;

    bool deleteDir(const QUrl &url)
    {
        QSharedPointer<dfmio::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
        if (!factory) {
            return false;
        }

        QSharedPointer<dfmio::DEnumerator> enumerator = factory->createEnumerator();
        if (!enumerator) {
            return false;
        }

        bool succ = false;
        while (enumerator->hasNext()) {
            const QString &path = enumerator->next();

            const QUrl &urlNext = QUrl::fromLocalFile(path);
            DecoratorFileInfo info(urlNext);
            if (info.isDir()) {
                succ = deleteDir(urlNext);
            } else {
                succ = deleteFile(urlNext);
            }
        }
        succ = deleteFile(url);
        return succ;
    }
    bool deleteFile(const QUrl &url)
    {
        if (url == this->url) {
            if (doperator)
                return doperator->deleteFile();
        } else {
            DecoratorFileOperator fileOperator(url);
            fileOperator.deleteFile();
        }
        return false;
    }

public:
    DecoratorFileOperator *q = nullptr;
    QSharedPointer<DFMIO::DOperator> doperator = nullptr;
    QUrl url;
};

DFMBASE_END_NAMESPACE

DFMBASE_USE_NAMESPACE

DecoratorFileOperator::DecoratorFileOperator(const QString &filePath)
    : d(new DecoratorFileOperatorPrivate(this))
{
    d->url = QUrl::fromLocalFile(filePath);
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(d->url.scheme(), static_cast<QUrl>(d->url));
    if (factory)
        d->doperator = factory->createOperator();
}

DecoratorFileOperator::DecoratorFileOperator(const QUrl &url)
    : d(new DecoratorFileOperatorPrivate(this))
{
    d->url = url;
    QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(url.scheme(), static_cast<QUrl>(url));
    if (factory)
        d->doperator = factory->createOperator();
}

DecoratorFileOperator::DecoratorFileOperator(QSharedPointer<dfmio::DOperator> dfileOperator)
    : d(new DecoratorFileOperatorPrivate(this))
{
    d->url = dfileOperator->uri();
    d->doperator = dfileOperator;
}

QSharedPointer<dfmio::DOperator> DecoratorFileOperator::operatorPtr()
{
    return d->doperator;
}

bool DecoratorFileOperator::deleteFile()
{
    DecoratorFileInfo fileInfo(d->url);
    if (fileInfo.isDir()) {
        return d->deleteDir(d->url);
    } else {
        return d->deleteFile(d->url);
    }
}

DFMIOError DecoratorFileOperator::lastError() const
{
    if (d->doperator)
        return d->doperator->lastError();
    return DFMIOError();
}
