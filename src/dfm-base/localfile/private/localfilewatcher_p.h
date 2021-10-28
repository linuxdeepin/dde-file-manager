/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef LOCALFILEWATCHER_P_H
#define LOCALFILEWATCHER_P_H

#include "localfile/localfilewatcher.h"
#include "base/private/abstractfilewatcher_p.h"
#include "utils/threadcontainer.hpp"

#include <dfm-io/core/dwatcher.h>

#include <QUrl>

USING_IO_NAMESPACE
DFMBASE_BEGIN_NAMESPACE
class LocalFileWatcherPrivate : public AbstractFileWatcherPrivate
{
    friend class LocalFileWatcher;
    LocalFileWatcher *const q;
    QSharedPointer<DWatcher> watcher { nullptr };       // dfm-io的文件监视器
public:
    explicit LocalFileWatcherPrivate(LocalFileWatcher *qq);
    virtual ~LocalFileWatcherPrivate() {}
    virtual bool start();
    virtual bool stop();
    static QString formatPath(const QString &path);
    void initFileWatcher();
    void initConnect();
};
DFMBASE_END_NAMESPACE

#endif // LOCALFILEWATCHER_P_H
