// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
    if (watcher.isNull()) {
        qCWarning(logDFMBase) << "LocalFileWatcher::start: Cannot start watcher, watcher instance is null";
        return false;
    }

    dfmio::DFile file(url);
    if (!file.exists()) {
        qCWarning(logDFMBase) << "LocalFileWatcher::start: Failed to start watcher, target directory does not exist:" << url;
        return false;
    }

    qCDebug(logDFMBase) << "LocalFileWatcher::start: Starting file watcher for:" << url;
    started = watcher->start();
    if (!started) {
        qCWarning(logDFMBase) << "LocalFileWatcher::start: Failed to start watcher for:" << url
                              << "Error:" << watcher->lastError().errorMsg();
    } else {
        qCInfo(logDFMBase) << "LocalFileWatcher::start: Successfully started file watcher for:" << url;
    }
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
    if (watcher.isNull()) {
        qCWarning(logDFMBase) << "LocalFileWatcher::stop: Cannot stop watcher, watcher instance is null";
        return false;
    }

    qCDebug(logDFMBase) << "LocalFileWatcher::stop: Stopping file watcher for:" << url;
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
    qCDebug(logDFMBase) << "LocalFileWatcher::initFileWatcher: Initializing file watcher for:" << url;
    watcher.reset(new DWatcher(url));
    if (!watcher) {
        qCCritical(logDFMBase) << "LocalFileWatcher::initFileWatcher: Critical error - failed to create DWatcher instance for:" << url;
        abort();
    }
    qCDebug(logDFMBase) << "LocalFileWatcher::initFileWatcher: Successfully created DWatcher instance for:" << url;
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
    qCDebug(logDFMBase) << "LocalFileWatcher::notifyFileAdded: File added notification for:" << url;
    emit subfileCreated(url);
}

void LocalFileWatcher::notifyFileChanged(const QUrl &url)
{
    qCDebug(logDFMBase) << "LocalFileWatcher::notifyFileChanged: File changed notification for:" << url;
    emit fileAttributeChanged(url);
}

void LocalFileWatcher::notifyFileDeleted(const QUrl &url)
{
    qCDebug(logDFMBase) << "LocalFileWatcher::notifyFileDeleted: File deleted notification for:" << url;
    emit fileDeleted(url);
}

}
