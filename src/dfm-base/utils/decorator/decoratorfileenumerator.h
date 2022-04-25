/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
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

#ifndef DECORATORFILEENUMERATOR_H
#define DECORATORFILEENUMERATOR_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/denumerator.h>

DFMBASE_BEGIN_NAMESPACE

class DecoratorFileEnumeratorPrivate;
class DecoratorFileEnumerator
{
public:
    explicit DecoratorFileEnumerator(const QString &filePath,
                                     const QStringList &nameFilters = QStringList(),
                                     DFMIO::DEnumerator::DirFilters filters = DFMIO::DEnumerator::DirFilter::NoFilter,
                                     DFMIO::DEnumerator::IteratorFlags flags = DFMIO::DEnumerator::IteratorFlag::NoIteratorFlags);
    explicit DecoratorFileEnumerator(const QUrl &url,
                                     const QStringList &nameFilters = QStringList(),
                                     DFMIO::DEnumerator::DirFilters filters = DFMIO::DEnumerator::DirFilter::NoFilter,
                                     DFMIO::DEnumerator::IteratorFlags flags = DFMIO::DEnumerator::IteratorFlag::NoIteratorFlags);

    explicit DecoratorFileEnumerator(QSharedPointer<DFMIO::DEnumerator> dfileEnumerator);

    ~DecoratorFileEnumerator() = default;

public:
    QSharedPointer<DFMIO::DEnumerator> enumeratorPtr();

    bool isValid() const;
    bool hasNext() const;
    QString next() const;
    QUrl nextUrl() const;
    QSharedPointer<DFMIO::DFileInfo> fileInfo() const;

    DFMIOError lastError() const;

private:
    QSharedPointer<DecoratorFileEnumeratorPrivate> d = nullptr;
};

DFMBASE_END_NAMESPACE

#endif   // DECORATORFILEENUMERATOR_H
