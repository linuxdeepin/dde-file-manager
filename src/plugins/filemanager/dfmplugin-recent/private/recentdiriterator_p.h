/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef RECENTDIRITERATORPRIVATE_H
#define RECENTDIRITERATORPRIVATE_H

#include "dfmplugin_recent_global.h"
#include "dfm-base/interfaces/abstractdiriterator.h"

#include <QQueue>

DPRECENT_BEGIN_NAMESPACE

class RecentDirIterator;

class RecentDirIteratorPrivate
{
    friend class RecentDirIterator;

public:
    explicit RecentDirIteratorPrivate(RecentDirIterator *qq);
    ~RecentDirIteratorPrivate();

private:
    QUrl currentUrl;
    QQueue<QUrl> urlList;
    QMap<QUrl, AbstractFileInfoPointer> recentNodes;
    RecentDirIterator *q { nullptr };
};

DPRECENT_END_NAMESPACE
#endif   // RECENTDIRITERATORPRIVATE_H
