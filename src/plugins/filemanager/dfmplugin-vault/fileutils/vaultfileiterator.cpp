// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfileiterator.h"
#include "private/vaultfileiteratorprivate.h"
#include "utils/vaulthelper.h"
#include <dfm-base/base/schemefactory.h>

#include <QDirIterator>
#include <QSharedPointer>

DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

VaultFileIterator::VaultFileIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(VaultHelper::vaultToLocalUrl(url), nameFilters, filters, flags)
{
    QUrl localUrl = VaultHelper::vaultToLocalUrl(url);
    discIterator = QSharedPointer<QDirIterator>(new QDirIterator(localUrl.path(), nameFilters, filters, flags));
}

VaultFileIterator::~VaultFileIterator()
{
}

QUrl VaultFileIterator::next()
{
    if (discIterator && discIterator->hasNext()) {
        return VaultHelper::instance()->pathToVaultVirtualUrl(discIterator->next());
    }

    return QUrl();
}

bool VaultFileIterator::hasNext() const
{
    return discIterator->hasNext();
}

QString VaultFileIterator::fileName() const
{
    return discIterator->fileName();
}

QUrl VaultFileIterator::fileUrl() const
{
    QString path = discIterator->filePath();
    QUrl url = VaultHelper::instance()->pathToVaultVirtualUrl(path);
    return url;
}

const FileInfoPointer VaultFileIterator::fileInfo() const
{
    return InfoFactory::create<FileInfo>(fileUrl());
}

QUrl VaultFileIterator::url() const
{
    return VaultHelper::instance()->rootUrl();
}
