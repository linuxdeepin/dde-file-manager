// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "file/local/localfilewatcher.h"
#include "file/local/private/localfilewatcher_p.h"
#include <dfm-base/base/urlroute.h>

#include <dfm-io/dwatcher.h>

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

    dfmio::DFile file(url);
    if (!file.exists()) {
        qCWarning(logDFMBase) << "watcher start failed, error: watcher dir is not exists ! url = " << url;
        return false;
    }

    started = watcher->start();
    if (!started)
        qCWarning(logDFMBase) << "watcher start failed, error: " << watcher->lastError().errorMsg();
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
    watcher.reset(new DWatcher(url));
    if (!watcher) {
        qCWarning(logDFMBase, "watcher create failed.");
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
