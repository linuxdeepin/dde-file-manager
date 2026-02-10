// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREITERATOR_H
#define SHAREITERATOR_H

#include "dfmplugin_myshares_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>

namespace dfmplugin_myshares {

class ShareIteratorPrivate;
class ShareIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
    Q_OBJECT
    friend class ShareIteratorPrivate;

public:
    explicit ShareIterator(const QUrl &url,
                           const QStringList &nameFilters = QStringList(),
                           QDir::Filters filters = QDir::NoFilter,
                           QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    virtual ~ShareIterator() override;

    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const FileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;

private:
    QScopedPointer<ShareIteratorPrivate> d;
};

}

#endif   // SHAREITERATOR_H
