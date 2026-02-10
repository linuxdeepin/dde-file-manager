// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHDIRITERATOR_H
#define TRASHDIRITERATOR_H

#include "dfmplugin_trash_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>

namespace dfmplugin_trash {

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
    virtual const FileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;

private:
    QScopedPointer<TrashDirIteratorPrivate> d;
};

}
#endif   // TRASHDIRITERATOR_H
