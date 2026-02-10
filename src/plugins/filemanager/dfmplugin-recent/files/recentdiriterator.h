// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTDIRITERATOR_H
#define RECENTDIRITERATOR_H

#include "dfmplugin_recent_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>

namespace dfmplugin_recent {

class RecentDirIteratorPrivate;
class RecentDirIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
    Q_OBJECT
    friend class RecentDirIteratorPrivate;

public:
    explicit RecentDirIterator(const QUrl &url,
                               const QStringList &nameFilters = QStringList(),
                               QDir::Filters filters = QDir::NoFilter,
                               QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    ~RecentDirIterator() override;

    virtual QUrl next() override;

    virtual bool hasNext() const override;

    virtual QString fileName() const override;

    virtual QUrl fileUrl() const override;

    virtual const FileInfoPointer fileInfo() const override;

    virtual QUrl url() const override;

private:
    QScopedPointer<RecentDirIteratorPrivate> d;
};

}
#endif   // RECENTDIRITERATOR_H
