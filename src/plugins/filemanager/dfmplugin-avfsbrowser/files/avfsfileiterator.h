// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILEITERATOR_H
#define AVFSFILEITERATOR_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-base/file/local/localdiriterator.h>

namespace dfmplugin_avfsbrowser {

class AvfsFileIteratorPrivate;
class AvfsFileIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
    Q_OBJECT

public:
    explicit AvfsFileIterator(const QUrl &url,
                              const QStringList &nameFilters = QStringList(),
                              QDir::Filters filters = QDir::NoFilter,
                              QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    virtual ~AvfsFileIterator();
    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const FileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;

private:
    QScopedPointer<AvfsFileIteratorPrivate> d;
};

}

#endif   // AVFSFILEITERATOR_H
