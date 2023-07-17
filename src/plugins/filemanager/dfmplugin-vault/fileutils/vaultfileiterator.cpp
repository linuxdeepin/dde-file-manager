// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfileiterator.h"
#include "private/vaultfileiteratorprivate.h"
#include "utils/vaulthelper.h"

#include <dfm-base/base/schemefactory.h>

#include <dfm-io/denumerator.h>
#include <dfm-io/dfmio_utils.h>

#include <QSharedPointer>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;
USING_IO_NAMESPACE

VaultFileIterator::VaultFileIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(VaultHelper::vaultToLocalUrl(url), nameFilters, filters, flags)
{
    QUrl localUrl = VaultHelper::vaultToLocalUrl(url);
    dfmioDirIterator.reset(new DFMIO::DEnumerator(localUrl, nameFilters,
                                                  static_cast<DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                                  static_cast<DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags))));
    if (!dfmioDirIterator)
        qCritical("Vault: create DEnumerator failed!");
}

VaultFileIterator::~VaultFileIterator()
{
}

QUrl VaultFileIterator::next()
{
    if (dfmioDirIterator)
        currentUrl = VaultHelper::instance()->pathToVaultVirtualUrl(dfmioDirIterator->next().path());

    return currentUrl;
}

bool VaultFileIterator::hasNext() const
{
    return (dfmioDirIterator ? dfmioDirIterator->hasNext() : false);
}

QString VaultFileIterator::fileName() const
{
    return fileUrl().fileName();
}

QUrl VaultFileIterator::fileUrl() const
{
    return currentUrl;
}

const FileInfoPointer VaultFileIterator::fileInfo() const
{
    return InfoFactory::create<FileInfo>(fileUrl());
}

QUrl VaultFileIterator::url() const
{
    return VaultHelper::instance()->rootUrl();
}
