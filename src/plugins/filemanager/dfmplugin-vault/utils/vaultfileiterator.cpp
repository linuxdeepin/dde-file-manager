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
#include "vaultfileiterator.h"
#include "private/vaultfileiteratorprivate.h"
#include "utils/vaulthelper.h"
#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

VaultFileIterator::VaultFileIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : LocalDirIterator(QUrl::fromLocalFile(url.path().contains(VaultHelper::rootUrl().path()) ? url.path() : UrlRoute::urlToPath(url)), nameFilters, filters, flags)
{
}

VaultFileIterator::~VaultFileIterator()
{
}

QUrl VaultFileIterator::next()
{
    QUrl url = LocalDirIterator::next();
    url.setScheme(VaultHelper::scheme());
    return url;
}

bool VaultFileIterator::hasNext() const
{
    return LocalDirIterator::hasNext();
}

QString VaultFileIterator::fileName() const
{
    return LocalDirIterator::fileName();
}

QUrl VaultFileIterator::fileUrl() const
{
    return LocalDirIterator::fileUrl();
}

const AbstractFileInfoPointer VaultFileIterator::fileInfo() const
{
    return InfoFactory::create<AbstractFileInfo>(fileUrl());
}

QUrl VaultFileIterator::url() const
{
    return LocalDirIterator::url();
}
