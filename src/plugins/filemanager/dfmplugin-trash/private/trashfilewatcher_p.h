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
#include "dfm-base/interfaces/private/abstractfilewatcher_p.h"

DPTRASH_BEGIN_NAMESPACE

class TrashFileWatcher;
class TrashFileWatcherPrivate : public DFMBASE_NAMESPACE::AbstractFileWatcherPrivate
{
    friend TrashFileWatcher;

public:
    TrashFileWatcherPrivate(const QUrl &fileUrl, TrashFileWatcher *qq);

public:
    bool start() override;
    bool stop() override;

    void initFileWatcher();
    void initConnect();

    AbstractFileWatcherPointer proxy;
    QMap<QUrl, AbstractFileWatcherPointer> urlToWatcherMap;
};

DPTRASH_END_NAMESPACE
#endif   // TRASHDIRITERATORPRIVATE_H
