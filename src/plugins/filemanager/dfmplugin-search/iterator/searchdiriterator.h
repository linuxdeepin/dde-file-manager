/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHDIRITERATOR_H
#define SEARCHDIRITERATOR_H

#include "dfmplugin_search_global.h"

#include "dfm-base/interfaces/abstractdiriterator.h"

DFMBASE_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

class SearchDirIteratorPrivate;
class SearchDirIterator : public AbstractDirIterator
{
    Q_OBJECT
    friend class SearchDirIteratorPrivate;

public:
    explicit SearchDirIterator(const QUrl &url,
                               const QStringList &nameFilters = QStringList(),
                               dfmio::DEnumerator::DirFilters filters = dfmio::DEnumerator::DirFilter::NoFilter,
                               dfmio::DEnumerator::IteratorFlags flags = dfmio::DEnumerator::IteratorFlag::NoIteratorFlags);
    ~SearchDirIterator() override;
    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const AbstractFileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;

private:
    SearchDirIteratorPrivate *const d { nullptr };
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCHDIRITERATOR_H
