// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "decoratorfileoperator.h"
#include "decoratorfileinfo.h"

#include <dfm-io/denumerator.h>

namespace dfmbase {

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
        QSharedPointer<DFMIO::DEnumerator> enumerator { new DFMIO::DEnumerator(url) };
        if (!enumerator) {
            return false;
        }

        bool succ = false;
        while (enumerator->hasNext()) {
            const QUrl &urlNext = enumerator->next();

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

}

using namespace dfmbase;

DecoratorFileOperator::DecoratorFileOperator(const QString &filePath)
    : d(new DecoratorFileOperatorPrivate(this))
{
    d->url = QUrl::fromLocalFile(filePath);
    d->doperator.reset(new DFMIO::DOperator(d->url));
}

DecoratorFileOperator::DecoratorFileOperator(const QUrl &url)
    : d(new DecoratorFileOperatorPrivate(this))
{
    d->url = url;
    d->doperator.reset(new DFMIO::DOperator(d->url));
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
