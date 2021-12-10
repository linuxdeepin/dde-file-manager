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
#include <dfmio_global.h>
#include <dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QEvent>
#include <QDir>
#include <QDebug>
#include <QApplication>

DFMBASE_BEGIN_NAMESPACE
/*!
 * \class AbstractFileWatcherPrivate 文件监视器私有类
 *
 * \brief 存储DAbstractFileWatcher的使用的变量和数据
 */
LocalFileWatcherPrivate::LocalFileWatcherPrivate(LocalFileWatcher *qq)
    : AbstractFileWatcherPrivate(qq), q(qq)
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
    if (!watcher)
        return false;
    started = watcher->start(0);
    if (started)
        watcherPath.append(path);
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
    if (!watcher)
        return false;
    started = watcher->stop();
    if (started)
        watcherPath.removeOne(path);
    return started;
}

LocalFileWatcher::~LocalFileWatcher()
{
    stopWatcher();
    if (d->watcher)
        disconnect(d->watcher.data());
}
/*!
 * \brief formatPath 获取文件的绝对路径
 *
 * 通过QFileInfo来获取绝对路径
 *
 * \return QString 文件的绝对路径
 */
QString LocalFileWatcherPrivate::formatPath(const QString &path)
{
    QString p = QFileInfo(path).absoluteFilePath();

    if (p.endsWith(QDir::separator()))
        p.chop(1);

    return p.isEmpty() ? path : p;
}

LocalFileWatcher::LocalFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(url, parent), d(new LocalFileWatcherPrivate(this))
{
    d->path = LocalFileWatcherPrivate::formatPath(UrlRoute::urlToPath(url));
    if (!UrlRoute::isVirtual(url)) {
        d->initFileWatcher();
        d->initConnect();
    }
}
/*!
 * \brief url 获取监视文件的url
 *
 * \return QUrl 文件的绝对路径
 */
QUrl LocalFileWatcher::url() const
{
    return d->url;
}
/*!
 * \brief startWatcher 启动文件监视器（在主线程中执行）
 *
 * \return bool 启动文件监视是否成功
 */
bool LocalFileWatcher::startWatcher()
{
    if (d->started)
        return true;

    QObject::moveToThread(qApp->thread());
    if (d->start()) {
        d->started = true;
        return true;
    }

    return false;
}
/*!
 * \brief stopWatcher 停止文件监视器（在主线程中执行）
 *
 * \return bool 停止文件监视是否成功
 */
bool LocalFileWatcher::stopWatcher()
{
    if (!d->started)
        return false;

    if (d->stop()) {
        d->started = false;
        return true;
    }

    return false;
}
/*!
 * \brief restartWatcher 重新启动文件监视器（在主线程中执行）
 *
 * 实际是先停止在启动，那么就是停止和启动都要成功，当监视器未启动时，调用返回失败，停止失败，但是文件监视器已启动
 *
 * \return bool 重启文件监视是否成功
 */
bool LocalFileWatcher::restartWatcher()
{
    bool ok = stopWatcher();
    return ok && startWatcher();
}
/*!
 * \brief setEnabledSubfileWatcher 设置当前目录下的子文件是否可以监视
 *
 * \param const QUrl &subfileUrl 当前目录下的子文件的URL
 *
 * \param bool enabled 是否能监视
 */
void LocalFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
    Q_UNUSED(subfileUrl)
    Q_UNUSED(enabled)
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
    QObject::connect(watcher.data(), &DWatcher::fileChanged, q, &LocalFileWatcher::fileAttributeChanged);
    QObject::connect(watcher.data(), &DWatcher::fileDeleted, q, &LocalFileWatcher::fileDeleted);
    QObject::connect(watcher.data(), &DWatcher::fileAdded, q, &LocalFileWatcher::subfileCreated);
}
DFMBASE_END_NAMESPACE
