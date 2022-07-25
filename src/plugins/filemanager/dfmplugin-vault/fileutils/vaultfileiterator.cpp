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

#include <QDirIterator>
#include <QSharedPointer>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

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

const AbstractFileInfoPointer VaultFileIterator::fileInfo() const
{
    return InfoFactory::create<AbstractFileInfo>(fileUrl());
}

QUrl VaultFileIterator::url() const
{
    return VaultHelper::instance()->rootUrl();
}
