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
#ifndef VAULTFILEDIRITERATOR_H
#define VAULTFILEDIRITERATOR_H

#include "dfmplugin_vault_global.h"
#include "dfm-base/file/local/localdiriterator.h"

DPVAULT_BEGIN_NAMESPACE
class VaultFileIterator : public DFMBASE_NAMESPACE::LocalDirIterator
{
    Q_OBJECT
    friend class VaultFileIteratorPrivate;

public:
    explicit VaultFileIterator(const QUrl &url,
                               const QStringList &nameFilters = QStringList(),
                               QDir::Filters filters = QDir::NoFilter,
                               QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    virtual ~VaultFileIterator() override;

    virtual QUrl next() override;

    virtual bool hasNext() const override;

    virtual QString fileName() const override;

    virtual QUrl fileUrl() const override;

    virtual const AbstractFileInfoPointer fileInfo() const override;

    virtual QUrl url() const override;
};
DPVAULT_END_NAMESPACE
#endif   // VAULTFILEDIRITERATOR_H
