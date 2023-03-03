// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DECORATORFILEENUMERATOR_H
#define DECORATORFILEENUMERATOR_H

#include "dfm-base/dfm_base_global.h"

#include <dfm-io/core/denumerator.h>

namespace dfmbase {

class DecoratorFileEnumeratorPrivate;
class DecoratorFileEnumerator
{
public:
    explicit DecoratorFileEnumerator(const QString &filePath,
                                     const QStringList &nameFilters = QStringList(),
                                     DFMIO::DEnumerator::DirFilters filters = DFMIO::DEnumerator::DirFilter::kNoFilter,
                                     DFMIO::DEnumerator::IteratorFlags flags = DFMIO::DEnumerator::IteratorFlag::kNoIteratorFlags);
    explicit DecoratorFileEnumerator(const QUrl &url,
                                     const QStringList &nameFilters = QStringList(),
                                     DFMIO::DEnumerator::DirFilters filters = DFMIO::DEnumerator::DirFilter::kNoFilter,
                                     DFMIO::DEnumerator::IteratorFlags flags = DFMIO::DEnumerator::IteratorFlag::kNoIteratorFlags);

    explicit DecoratorFileEnumerator(QSharedPointer<DFMIO::DEnumerator> dfileEnumerator);

    ~DecoratorFileEnumerator() = default;

public:
    QSharedPointer<DFMIO::DEnumerator> enumeratorPtr();

    bool isValid() const;
    bool hasNext() const;
    QUrl next() const;
    QUrl nextUrl() const;
    quint64 fileCount() const;
    QSharedPointer<DFMIO::DFileInfo> fileInfo() const;

    DFMIOError lastError() const;

private:
    QSharedPointer<DecoratorFileEnumeratorPrivate> d = nullptr;
};

}

#endif   // DECORATORFILEENUMERATOR_H
