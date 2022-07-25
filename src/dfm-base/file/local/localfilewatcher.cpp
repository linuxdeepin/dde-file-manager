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
#include "file/local/localfilewatcher.h"
#include "file/local/private/localfilewatcher_p.h"
#include "base/urlroute.h"

#include <dfm-io/core/dwatcher.h>
#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QEvent>
#include <QDir>
#include <QDebug>
#include <QApplication>

namespace dfmbase {
/*!
 * \class AbstractFileWatcherPrivate 文件监视器私有类
 *
 * \brief 存储DAbstractFileWatcher的使用的变量和数据
 */
LocalFileWatcherPrivate::LocalFileWatcherPrivate(const QUrl &fileUrl, LocalFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}
/*!
 * \brief start 启动文件监视器
 *
 * DAbstractFileWatcher启动监视器时使用
 *
 * \return 返回是否启动成功
 */
bool LocalFileWatcherPrivate::start()
{
    if (watcher.isNull())
        return false;
    started = watcher->start(0);
    if (started)
        watcherPath.appendByLock(path);
    return started;
}
/*!
 * \brief stop 停止文件监视器
 *
 * DAbstractFileWatcher停止监视器时使用
 *
 * \return 返回是否停止成功
 */
bool LocalFileWatcherPrivate::stop()
{
    if (watcher.isNull())
        return false;
    started = watcher->stop();
    if (started)
        watcherPath.removeOneByLock(path);
    return started;
}

LocalFileWatcher::~LocalFileWatcher()
{
}

LocalFileWatcher::LocalFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new LocalFileWatcherPrivate(url, this), parent)
{
    LocalFileWatcherPrivate *dptr = static_cast<LocalFileWatcherPrivate *>(d.data());
    dptr->initFileWatcher();
    dptr->initConnect();
}

/*!
 * \brief AbstractFileWatcher::initFileWatcher 初始化和创建dfm-io的filewatcher
 */
void LocalFileWatcherPrivate::initFileWatcher()
{
    QUrl watchUrl = QUrl::fromLocalFile(path);
    QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(watchUrl.scheme(), static_cast<QUrl>(watchUrl));
    if (!factory) {
        qWarning("create factory failed.");
        abort();
    }

    watcher = factory->createWatcher();
    if (!watcher) {
        qWarning("watcher create failed.");
        abort();
    }
}
/*!
 * \brief AbstractFileWatcher::initConnect 初始化dfm-io中文件监视器的信号连接
 */
void LocalFileWatcherPrivate::initConnect()
{
    connect(watcher.data(), &DWatcher::fileChanged, q, &AbstractFileWatcher::fileAttributeChanged);
    connect(watcher.data(), &DWatcher::fileDeleted, q, &AbstractFileWatcher::fileDeleted);
    connect(watcher.data(), &DWatcher::fileAdded, q, &AbstractFileWatcher::subfileCreated);
    connect(watcher.data(), &DWatcher::fileRenamed, q, &AbstractFileWatcher::fileRename);
}

void LocalFileWatcher::notifyFileAdded(const QUrl &url)
{
    emit subfileCreated(url);
}

void LocalFileWatcher::notifyFileChanged(const QUrl &url)
{
    emit fileAttributeChanged(url);
}

void LocalFileWatcher::notifyFileDeleted(const QUrl &url)
{
    emit fileDeleted(url);
}

}
