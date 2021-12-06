/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#ifndef LOCALDIRITERATOR_H
#define LOCALDIRITERATOR_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/base/abstractdiriterator.h"

#include <QDirIterator>
#include <QSharedPointer>

class QUrl;
DFMBASE_BEGIN_NAMESPACE
class LocalFileInfo;
class LocalDirIteratorPrivate;

class LocalDirIterator : public AbstractDirIterator
{
    QScopedPointer<LocalDirIteratorPrivate> d;

public:
    explicit LocalDirIterator(const QUrl &url,
                              const QStringList &nameFilters = QStringList(),
                              QDir::Filters filters = QDir::NoFilter,
                              QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);
    virtual ~LocalDirIterator() override;
    virtual QUrl next() override;
    virtual bool hasNext() const override;
    virtual void close() override {}
    virtual QString fileName() const override;
    virtual QUrl fileUrl() const override;
    virtual const AbstractFileInfoPointer fileInfo() const override;
    virtual QUrl url() const override;
    bool enableIteratorByKeyword(const QString &keyword) override
    {
        Q_UNUSED(keyword);
        return false;
    }
};
DFMBASE_END_NAMESPACE

#endif   // LOCALDIRITERATOR_H
