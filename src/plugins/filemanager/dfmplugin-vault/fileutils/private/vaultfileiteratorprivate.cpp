// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfileiteratorprivate.h"
#include "fileutils/vaultfileiterator.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-io/denumerator.h>

USING_IO_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultFileIteratorPrivate::VaultFileIteratorPrivate(const QUrl &url,
                                                   const QStringList &nameFilters,
                                                   QDir::Filters filters,
                                                   QDirIterator::IteratorFlags flags,
                                                   VaultFileIterator *qp)
    : q(qp), curFilters(filters)
{
    Q_UNUSED(nameFilters);
    Q_UNUSED(flags);
    fmDebug() << "Vault: Creating file iterator for URL:" << url.toString();
    QUrl temp = QUrl::fromLocalFile(UrlRoute::urlToPath(url));
    temp.setScheme(url.scheme());

    dfmioDirIterator.reset(new DEnumerator(temp));
    if (!dfmioDirIterator) {
        fmWarning("Vault: create enumerator failed!");
        abort();
    }
    fmDebug() << "Vault: File iterator created successfully";
}

VaultFileIteratorPrivate::~VaultFileIteratorPrivate()
{
    fmDebug() << "Vault: File iterator destroyed";
}
