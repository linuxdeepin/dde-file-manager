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
#ifndef LOCALFILEDIRITERATOR_P_H
#define LOCALFILEDIRITERATOR_P_H

#include "localfile/localdiriterator.h"
#include "base/urlroute.h"

#include <QDirIterator>
#include <QDebug>

#include <dfm-io/core/denumerator.h>
#include <dfmio_global.h>
#include <dfmio_register.h>
#include <dfm-io/core/diofactory.h>

USING_IO_NAMESPACE
DFMBASE_BEGIN_NAMESPACE
class LocalDirIteratorPrivate
{
    Q_DECLARE_PUBLIC(LocalDirIterator)
    class LocalDirIterator * const q_ptr;

public:
    explicit LocalDirIteratorPrivate(const QUrl &url,
                                       const QStringList &nameFilters,
                                       QDir::Filters filters,
                                       QDirIterator::IteratorFlags flags,
                                       LocalDirIterator * q)
        : q_ptr(q),
          url(url)
    {
        Q_UNUSED(nameFilters);
        Q_UNUSED(filters);
        Q_UNUSED(flags);
        QUrl temp = QUrl::fromLocalFile(UrlRoute::urlToPath(url));
        QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(temp.scheme(), temp);
        if (!factory) {
            qWarning("create factory failed.");
            return;
        }

        dfmioDirIterator = factory->createEnumerator();
        if (!dfmioDirIterator) {
            qWarning("create enumerator failed.");
        }
    }
private:
    QSharedPointer<dfmio::DEnumerator> dfmioDirIterator = nullptr; // dfmio的文件迭代器
    QUrl url; // 当前的url
};
DFMBASE_END_NAMESPACE
#endif // ABSTRACTDIRITERATOR_P_H
