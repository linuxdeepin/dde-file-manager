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
#include "base/abstractfilewatcher.h"
#include "base/private/abstractfilewatcher_p.h"
#include "localfile/filesignalmanager.h"
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
DThreadList<QString> AbstractFileWatcherPrivate::watcherPath;
/*!
 * \class AbstractFileWatcherPrivate 文件监视器私有类
 *
 * \brief 存储DAbstractFileWatcher的使用的变量和数据
 */
AbstractFileWatcherPrivate::AbstractFileWatcherPrivate(AbstractFileWatcher *qq)
    : QObject (qq)
    , q(qq)
{

}
/*!
 * \brief start 启动文件监视器
 *
 * DAbstractFileWatcher启动监视器时使用
 *
 * \return 返回是否启动成功
 */
bool AbstractFileWatcherPrivate::start()
{
    return started;
}
/*!
 * \brief stop 停止文件监视器
 *
 * DAbstractFileWatcher停止监视器时使用
 *
 * \return 返回是否停止成功
 */
bool AbstractFileWatcherPrivate::stop()
{
    return started;
}

AbstractFileWatcher::~AbstractFileWatcher()
{
    stopWatcher();
}
/*!
 * \brief formatPath 获取文件的绝对路径
 *
 * 通过QFileInfo来获取绝对路径
 *
 * \return QString 文件的绝对路径
 */
QString AbstractFileWatcherPrivate::formatPath(const QString &path)
{
    QString p = QFileInfo(path).absoluteFilePath();

    if (p.endsWith(QDir::separator()))
        p.chop(1);

    return p.isEmpty() ? path : p;
}
/*!
 * \class DAbstractFileWatcher 文件监视器类
 *
 * \brief 负责文件的监视，通过不同的url监视不同的文件和目录
 */
AbstractFileWatcher::AbstractFileWatcher(const QUrl &url, QObject *parent)
    : QObject (parent)
    , d(new AbstractFileWatcherPrivate(this))
{
    d->path = AbstractFileWatcherPrivate::formatPath(UrlRoute::urlToPath(url));
}
/*!
 * \brief url 获取监视文件的url
 *
 * \return QUrl 文件的绝对路径
 */
QUrl AbstractFileWatcher::url() const
{
    return d->url;
}
/*!
 * \brief startWatcher 启动文件监视器（在主线程中执行）
 *
 * \return bool 启动文件监视是否成功
 */
bool AbstractFileWatcher::startWatcher()
{
    return false;
}
/*!
 * \brief stopWatcher 停止文件监视器（在主线程中执行）
 *
 * \return bool 停止文件监视是否成功
 */
bool AbstractFileWatcher::stopWatcher()
{
    return false;
}
/*!
 * \brief restartWatcher 重新启动文件监视器（在主线程中执行）
 *
 * 实际是先停止在启动，那么就是停止和启动都要成功，当监视器未启动时，调用返回失败，停止失败，但是文件监视器已启动
 *
 * \return bool 重启文件监视是否成功
 */
bool AbstractFileWatcher::restartWatcher()
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
void AbstractFileWatcher::setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled)
{
    Q_UNUSED(subfileUrl)
    Q_UNUSED(enabled)
}

DFMBASE_END_NAMESPACE
