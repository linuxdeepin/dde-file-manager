// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBSHAREITERATOR_H
#define SMBSHAREITERATOR_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>

namespace dfmplugin_smbbrowser {

class SmbShareIteratorPrivate;
class SmbShareIterator : public dfmbase::AbstractDirIterator
{
    Q_OBJECT
    friend class SmbShareIteratorPrivate;

public:
    explicit SmbShareIterator(const QUrl &url,
                              const QStringList &nameFilters = QStringList(),
                              QDir::Filters filters = QDir::NoFilter,
                              QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    virtual ~SmbShareIterator() override;

    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const FileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;
    bool initIterator() override;

private:
    QScopedPointer<SmbShareIteratorPrivate> d;
};

}

#endif   // SMBSHAREITERATOR_H
