/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#include "localfile/private/dfmlocaldiriterator_p.h"
#include "localfile/dfmlocalfileinfo.h"
#include "localfile/dfmlocaldiriterator.h"
#include "base/dfmurlroute.h"

DFMLocalDirIterator::DFMLocalDirIterator(const QUrl &url,
                                       const QStringList &nameFilters,
                                       QDir::Filters filters,
                                       QDirIterator::IteratorFlags flags)
    : d_ptr(new DFMLocalDirIteratorPrivate(url,nameFilters, filters,flags,this))
{

}

DFMLocalDirIterator::~DFMLocalDirIterator()
{

}

QUrl DFMLocalDirIterator::next()
{
    Q_D(DFMLocalDirIterator);
    return DFMUrlRoute::pathToUrl(d->m_dirIterator.next());
}

bool DFMLocalDirIterator::hasNext() const
{
    Q_D(const DFMLocalDirIterator);
    return d->m_dirIterator.hasNext();
}

QString DFMLocalDirIterator::fileName() const
{
    Q_D(const DFMLocalDirIterator);
    return d->m_dirIterator.fileName();
}

QUrl DFMLocalDirIterator::fileUrl() const
{
    Q_D(const DFMLocalDirIterator);
    return DFMUrlRoute::pathToUrl(d->m_dirIterator.filePath());
}

const DAbstractFileInfoPointer DFMLocalDirIterator::fileInfo() const
{
    return DAbstractFileInfoPointer(new DFMLocalFileInfo(fileUrl()));
}

QUrl DFMLocalDirIterator::url() const
{
    Q_D(const DFMLocalDirIterator);
    return DFMUrlRoute::pathToUrl(d->m_dirIterator.path());
}
