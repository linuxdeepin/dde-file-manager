/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMLOCALDIRITERATOR_H
#define DFMLOCALDIRITERATOR_H

#include "dfm-base/base/dabstractdiriterator.h"

#include "dfm-base/localfile/dfmlocalfileinfo.h"

#include <QDirIterator>

class QUrl;
class DFMLocalFileInfo;
class DFMLocalDirIteratorPrivate;

class DFMLocalDirIterator : public DAbstractDirIterator
{
    Q_DECLARE_PRIVATE(DFMLocalDirIterator)
    DFMLocalDirIteratorPrivate * const d_ptr;

public:
    explicit DFMLocalDirIterator(const QUrl &url,
                                 const QStringList &nameFilters = QStringList(),
                                 QDir::Filters filters = QDir::NoFilter,
                                 QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    ~DFMLocalDirIterator() override;

    virtual QUrl next() override;

    virtual bool hasNext() const override;

    virtual void close() override {}

    virtual QString fileName() const override;

    // Returns the full file url for the current directory entry.
    virtual QUrl fileUrl() const override;

    virtual const DAbstractFileInfoPointer fileInfo() const override;

    // Returns the base url of the iterator.
    virtual QUrl url() const override;

    bool enableIteratorByKeyword(const QString &keyword) override {Q_UNUSED(keyword); return false;}
};

#endif // DFMLOCALDIRITERATOR_H
