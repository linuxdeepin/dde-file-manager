// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTEREDMEDIADIRITERATOR_H
#define MASTEREDMEDIADIRITERATOR_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/abstractdiriterator.h>

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
    QSharedPointer<QDirIterator> discIterator;
    QSharedPointer<QDirIterator> stagingIterator;
    QString mntPoint;
    QString devFile;
    QSet<QString> seen;
    QSet<QUrl> skip;
    QUrl changeScheme(const QUrl &in) const;
    QUrl changeSchemeUpdate(const QUrl &in);
};

}

#endif   // MASTEREDMEDIADIRITERATOR_H
