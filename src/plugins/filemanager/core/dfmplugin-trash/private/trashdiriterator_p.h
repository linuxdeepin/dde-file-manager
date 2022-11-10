/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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

#ifndef TRASHDIRITERATORPRIVATE_H
#define TRASHDIRITERATORPRIVATE_H

#include "dfmplugin_trash_global.h"
#include "dfm-base/interfaces/abstractdiriterator.h"

#include <dfm-io/core/denumerator.h>

#include <QDirIterator>

namespace dfmplugin_trash {

class TrashDirIterator;

class TrashDirIteratorPrivate
{
    friend class TrashDirIterator;

public:
    TrashDirIteratorPrivate(const QUrl &url, const QStringList &nameFilters,
                            DFMIO::DEnumerator::DirFilters filters, DFMIO::DEnumerator::IteratorFlags flags,
                            TrashDirIterator *qq);
    ~TrashDirIteratorPrivate();

private:
    TrashDirIterator *q { nullptr };
    QSharedPointer<DFMIO::DEnumerator> dEnumerator = nullptr;
    QUrl currentUrl;
    QMap<QString, QString> fstabMap;
};

}
#endif   // TRASHDIRITERATORPRIVATE_H
