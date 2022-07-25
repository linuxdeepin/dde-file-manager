/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef SEARCHFILEWATCHER_P_H
#define SEARCHFILEWATCHER_P_H

#include "dfmplugin_search_global.h"

#include "dfm-base/interfaces/private/abstractfilewatcher_p.h"

DFMBASE_USE_NAMESPACE
namespace dfmplugin_search {

class SearchFileWatcher;
class SearchFileWatcherPrivate : public AbstractFileWatcherPrivate
{
    friend class SearchFileWatcher;

public:
    explicit SearchFileWatcherPrivate(const QUrl &fileUrl, SearchFileWatcher *qq);

private:
    bool start() override;
    bool stop() override;

    QHash<QUrl, AbstractFileWatcherPointer> urlToWatcherHash;
};

}

#endif   // SEARCHFILEWATCHER_P_H
