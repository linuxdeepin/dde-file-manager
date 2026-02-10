// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTEREDMEDIADIRITERATOR_H
#define MASTEREDMEDIADIRITERATOR_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-io/denumerator.h>

#include <QSet>

namespace dfmplugin_optical {

class MasteredMediaDirIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
public:
    explicit MasteredMediaDirIterator(const QUrl &url,
                                      const QStringList &nameFilters,
                                      QDir::Filters filters,
                                      QDirIterator::IteratorFlags flags);

    QUrl next() override;
    bool hasNext() const override;
    QString fileName() const override;
    QUrl fileUrl() const override;
    virtual const FileInfoPointer fileInfo() const override;
    QUrl url() const override;

private:
    mutable QSharedPointer<dfmio::DEnumerator> discIterator { nullptr };
    QSharedPointer<dfmio::DEnumerator> stagingIterator { nullptr };
    QString mntPoint;
    QString devFile;
    QUrl currentUrl;   // 当前迭代器所在位置文件的url
    QUrl changeScheme(const QUrl &in) const;
};

}

#endif   // MASTEREDMEDIADIRITERATOR_H
