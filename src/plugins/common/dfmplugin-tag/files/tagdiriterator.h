// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGDIRITERATOR_H
#define TAGDIRITERATOR_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>

namespace dfmplugin_tag {

class TagDirIteratorPrivate;
class TagDirIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
    Q_OBJECT
    friend class TagDirIteratorPrivate;

public:
    explicit TagDirIterator(const QUrl &url,
                            const QStringList &nameFilters = QStringList(),
                            QDir::Filters filters = QDir::NoFilter,
                            QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    ~TagDirIterator() override;

    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const FileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;

private:
    QScopedPointer<TagDirIteratorPrivate> d;
};

}

#endif   // TAGDIRITERATOR_H
