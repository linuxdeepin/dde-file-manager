/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef ABSTRACTFILEWATCHER_P_H
#define ABSTRACTFILEWATCHER_P_H

#include "interfaces/abstractfilewatcher.h"
#include "utils/threadcontainer.hpp"

#include <dfm-io/core/dwatcher.h>

#include <QUrl>

USING_IO_NAMESPACE
namespace dfmbase {
class AbstractFileWatcherPrivate : public QObject
{
    Q_DISABLE_COPY(AbstractFileWatcherPrivate)
    friend class AbstractFileWatcher;

public:
    explicit AbstractFileWatcherPrivate(const QUrl &fileUrl, AbstractFileWatcher *qq);
    virtual ~AbstractFileWatcherPrivate() {}
    virtual bool start();
    virtual bool stop();
    static QString formatPath(const QString &path);

protected:
    AbstractFileWatcher *q = nullptr;
    QAtomicInteger<bool> started { false };   // 是否开始监视
    QAtomicInt cacheInfoConnectSize { 0 };   // Cached FileInfo, number of connections monitored
    QUrl url;   // 监视文件的url
    QString path;   // 监视文件的路径
    static DThreadList<QString> watcherPath;   // 全局监视文件的监视列表
};
}

#endif   // DABSTRACTFILEWATCHER_P_H
