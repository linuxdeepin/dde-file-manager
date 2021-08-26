/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DABSTRACTDIRITERATOR_H
#define DABSTRACTDIRITERATOR_H

#include "dfm-base/base/dabstractfileinfo.h"

class DAbstractDirIterator
{
public:

    virtual ~DAbstractDirIterator() {}

    virtual QUrl next() = 0;

    virtual bool hasNext() const = 0;

    virtual void close() {}

    virtual QString fileName() const = 0;

    // Returns the full file url for the current directory entry.
    virtual QUrl fileUrl() const = 0;

    virtual const DAbstractFileInfoPointer fileInfo() const = 0;

    // Returns the base url of the iterator.
    virtual QUrl url() const = 0;

    virtual bool enableIteratorByKeyword(const QString &keyword) {Q_UNUSED(keyword); return false;}
};

typedef QSharedPointer<DAbstractDirIterator> DAbstractDirIteratorPointer;

Q_DECLARE_METATYPE(DAbstractDirIteratorPointer)

#endif // DABSTRACTDIRITERATOR_H
