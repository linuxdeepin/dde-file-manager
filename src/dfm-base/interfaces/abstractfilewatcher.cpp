// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/private/abstractfilewatcher_p.h>
#include <dfm-base/base/urlroute.h>

#include <dfm-io/dfileinfo.h>

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
AbstractFileWatcherPrivate::AbstractFileWatcherPrivate(const QUrl &fileUrl, AbstractFileWatcher *qq)
    : QObject(), q(qq)
{
    url = fileUrl;
    this->path = formatPath(UrlRoute::urlToPath(fileUrl));
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
AbstractFileWatcher::AbstractFileWatcher(AbstractFileWatcherPrivate *dptr, QObject *parent)
    : QObject(parent), d(dptr)
{
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
    if (d->started)
        return true;

    QObject::moveToThread(qApp->thread());
    d->started = d->start();
    return d->started;
}
/*!
 * \brief stopWatcher 停止文件监视器（在主线程中执行）
 *
 * \return bool 停止文件监视是否成功
 */
bool AbstractFileWatcher::stopWatcher()
{
    if (!d->started)
        return true;

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
/*!
 * \brief AbstractFileWatcher::getCachInfoConnectSize Gets the number of file information cache monitors
 * \return Number of file information cache monitors
 */
int AbstractFileWatcher::getCacheInfoConnectSize() const
{
    return d->cacheInfoConnectSize;
}
/*!
 * \brief AbstractFileWatcher::addCachInfoConnectSize Increase the number of file information cache monitoring
 */
void AbstractFileWatcher::addCacheInfoConnectSize()
{
    d->cacheInfoConnectSize++;
}
/*!
 * \brief AbstractFileWatcher::reduceCachInfoConnectSize Reduce the number of file information cache monitoring
 */
void AbstractFileWatcher::reduceCacheInfoConnectSize()
{
    d->cacheInfoConnectSize--;
}

void AbstractFileWatcher::notifyFileAdded(const QUrl &url)
{
    Q_UNUSED(url);
}

void AbstractFileWatcher::notifyFileChanged(const QUrl &url)
{
    Q_UNUSED(url);
}

void AbstractFileWatcher::notifyFileDeleted(const QUrl &url)
{
    Q_UNUSED(url);
}

}
