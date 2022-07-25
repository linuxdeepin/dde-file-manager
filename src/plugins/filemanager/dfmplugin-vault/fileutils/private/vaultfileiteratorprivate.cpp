/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "vaultfileiteratorprivate.h"
#include "fileutils/vaultfileiterator.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

#include <dfm-io/local/dlocalenumerator.h>

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
    QUrl temp = QUrl::fromLocalFile(UrlRoute::urlToPath(url));
    temp.setScheme(url.scheme());

    QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(temp.scheme(), static_cast<QUrl>(temp));
    if (!factory) {
        qWarning("Failed dfm-io create factory .");
        abort();
    }

    dfmioDirIterator = factory->createEnumerator();
    if (!dfmioDirIterator) {
        qWarning("Failed dfm-io use factory create enumerator");
        abort();
    }
}

VaultFileIteratorPrivate::~VaultFileIteratorPrivate()
{
}
