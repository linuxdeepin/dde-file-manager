/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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

#ifndef TRASHDIRITERATOR_H
#define TRASHDIRITERATOR_H

#include "dfmplugin_trash_global.h"

#include "dfm-base/interfaces/abstractdiriterator.h"

DPTRASH_BEGIN_NAMESPACE

class TrashDirIteratorPrivate;
class TrashDirIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
    Q_OBJECT
    friend class TrashDirIteratorPrivate;

public:
    explicit TrashDirIterator(const QUrl &url,
                              const QStringList &nameFilters = QStringList(),
                              QDir::Filters filters = QDir::NoFilter,
                              QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    ~TrashDirIterator() override;

    virtual QUrl next() override;

    virtual bool hasNext() const override;

    virtual QString fileName() const override;

    virtual QUrl fileUrl() const override;

    virtual const AbstractFileInfoPointer fileInfo() const override;

    virtual QUrl url() const override;

private:
    QScopedPointer<TrashDirIteratorPrivate> d;
};

DPTRASH_END_NAMESPACE
#endif   // TRASHDIRITERATOR_H
