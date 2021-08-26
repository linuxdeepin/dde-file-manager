/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMLOCALFILEDIRITERATOR_P_H
#define DFMLOCALFILEDIRITERATOR_P_H

#include "localfile/dfmlocaldiriterator.h"
#include "base/dfmurlroute.h"

#include <QDirIterator>

class DFMLocalDirIteratorPrivate
{
    Q_DECLARE_PUBLIC(DFMLocalDirIterator)
    class DFMLocalDirIterator * const q_ptr;

public:
    explicit DFMLocalDirIteratorPrivate(const QUrl &url,
                                       const QStringList &nameFilters,
                                       QDir::Filters filters,
                                       QDirIterator::IteratorFlags flags,
                                       DFMLocalDirIterator * q)
        : q_ptr(q),
          m_dirIterator(DFMUrlRoute::urlToPath(url), nameFilters, filters, flags),
          m_url(url)
    {

    }
private:
    QDirIterator m_dirIterator;
    QUrl m_url;
};

#endif // DFMABSTRACTDIRITERATOR_P_H
