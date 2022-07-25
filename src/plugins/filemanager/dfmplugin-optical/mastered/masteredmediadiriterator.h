/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef MASTEREDMEDIADIRITERATOR_H
#define MASTEREDMEDIADIRITERATOR_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/interfaces/abstractdiriterator.h"

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
    virtual const AbstractFileInfoPointer fileInfo() const override;
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
