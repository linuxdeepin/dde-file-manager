/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef SHAREITERATOR_H
#define SHAREITERATOR_H

#include "dfmplugin_myshares_global.h"

#include "dfm-base/interfaces/abstractdiriterator.h"

DPMYSHARES_BEGIN_NAMESPACE

class ShareIteratorPrivate;
class ShareIterator : public DFMBASE_NAMESPACE::AbstractDirIterator
{
    Q_OBJECT
    friend class ShareIteratorPrivate;

public:
    explicit ShareIterator(const QUrl &url,
                           const QStringList &nameFilters = QStringList(),
                           QDir::Filters filters = QDir::NoFilter,
                           QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    virtual ~ShareIterator() override;

    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const AbstractFileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;

private:
    QScopedPointer<ShareIteratorPrivate> d;
};

DPMYSHARES_END_NAMESPACE

#endif   // SHAREITERATOR_H
