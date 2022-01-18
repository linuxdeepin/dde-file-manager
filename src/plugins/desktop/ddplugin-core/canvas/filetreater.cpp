/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#include "filetreater.h"
#include "private/filetreater_p.h"
//#include "grid/canvasgrid.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/base/urlroute.h"

#include <QDir>
#include <QtConcurrent>
#include <QStandardPaths>

DSB_D_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class FileTreaterGlobal : public FileTreater{};
Q_GLOBAL_STATIC(FileTreaterGlobal, fileTreater)

FileTreaterPrivate::FileTreaterPrivate(FileTreater *q_ptr)
    : QObject (q_ptr)
    , q(q_ptr)
{

}

void FileTreaterPrivate::doFileDeleted(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl, EventType>(url, RmFile));
    }

    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileTreaterPrivate::dofileCreated(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl, EventType>(url, AddFile));
    }

    metaObject()->invokeMethod(this, "doWatcherEvent", Qt::QueuedConnection);
}

void FileTreaterPrivate::doFileUpdated(const QUrl &url)
{
    Q_UNUSED(url)
    // todo(wangcl)
}

void FileTreaterPrivate::doUpdateChildren(const QList<QUrl> &childrens)
{
    fileList.clear();
    fileMap.clear();
    QString errString;
    for (auto children : childrens) {

        auto itemInfo = dfmbase::InfoFactory::create<dfmbase::LocalFileInfo>(children, &errString);
        if (!itemInfo) {
            qInfo() << "create LocalFileInfo error: " << errString;
            continue;
        }
        if (itemInfo->isHidden()) {
            qInfo() << "file is hidden:" << itemInfo->path();
            continue;
        }
        fileList.append(children);
        fileMap.insert(children, itemInfo);
    }

    refreshedFlag = true;
    canRefreshFlag = true;

    emit q->fileRefreshed();
}

void FileTreaterPrivate::doWatcherEvent()
{
//    if (refreshedFlag)
//        return;

    if (processFileEventRuning)
        return;

    processFileEventRuning = true;
    while (checkFileEventQueue()) {
        QPair<QUrl, EventType> event;
        {
            QMutexLocker lk(&watcherEventMutex);
            event = watcherEvent.dequeue();
        }
        const QUrl &fileUrl = event.first;
        if (!fileUrl.isValid())
            continue;

        if (UrlRoute::urlParent(fileUrl) != rootUrl)
            continue;

        if (event.second == AddFile) {
            if (fileMap.contains(fileUrl))
                continue;

            QString errString;
            auto itemInfo = dfmbase::InfoFactory::create<dfmbase::LocalFileInfo>(fileUrl, true, &errString);
            if (!itemInfo) {
                qInfo() << "create LocalFileInfo error: " << errString;
                continue;
            }
            if (itemInfo->isHidden()) {
                qInfo() << "file is hidden:" << itemInfo->path();
                continue;
            }
            fileList.append(fileUrl);
            fileMap.insert(fileUrl, itemInfo);

            emit q->fileCreated(fileUrl);
        } else {
            fileList.removeOne(fileUrl);
            fileMap.remove(fileUrl);

            emit q->fileDeleted(fileUrl);
        }
    }
    processFileEventRuning = false;
}

bool FileTreaterPrivate::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    bool isEmptyQueue = watcherEvent.isEmpty();
    return !isEmptyQueue;
}

FileTreater::FileTreater(QObject *parent)
    : QObject(parent)
    , d(new FileTreaterPrivate(this))
{
    Q_ASSERT(thread() == qApp->thread());
}

FileTreater::~FileTreater()
{

}

FileTreater *FileTreater::instance()
{
    return fileTreater;
}

/*!
 * \brief 根据url获取对应的文件对象, \a url 文件路径
 * \return
 */
DFMLocalFileInfoPointer FileTreater::fileInfo(const QString &url)
{
    return d->fileMap.value(url);
}

/*!
 * \brief 根据index获取对应的文件对象, \a index 序号
 * \return
 */
DFMLocalFileInfoPointer FileTreater::fileInfo(int index)
{
    if (index >= 0 && index < fileCount()) {
        QUrl url(d->fileList.at(index));
        return d->fileMap.value(url);
    }

    return nullptr;
}

int FileTreater::indexOfChild(AbstractFileInfoPointer info)
{
    return d->fileList.indexOf(info->url());
}

/*!
 * \brief 初始化数据
 */
void FileTreater::init()
{
    d->rootUrl = QUrl::fromLocalFile(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    if (!d->rootUrl.isValid()) {
        qWarning() << "desktop url is invalid:" << d->rootUrl;
        return;
    }

    if (!d->watcher.isNull()) {
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileDeleted, d.data(), &FileTreaterPrivate::doFileDeleted);
        disconnect(d->watcher.data(), &AbstractFileWatcher::subfileCreated, d.data(), &FileTreaterPrivate::dofileCreated);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged, d.data(), &FileTreaterPrivate::doFileUpdated);
    }

    d->watcher = WacherFactory::create<AbstractFileWatcher>(d->rootUrl);
    if (!d->watcher.isNull()) {
        connect(d->watcher.data(), &AbstractFileWatcher::fileDeleted, d.data(), &FileTreaterPrivate::doFileDeleted);
        connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated, d.data(), &FileTreaterPrivate::dofileCreated);
        connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged, d.data(), &FileTreaterPrivate::doFileUpdated);
        d->watcher->startWatcher();
    }

    d->canRefreshFlag = true;
}

const QList<QUrl> &FileTreater::getFiles() const
{
    return d->fileList;
}

int FileTreater::fileCount() const
{
    return d->fileList.count();
}

QUrl FileTreater::rootUrl() const
{
    return d->rootUrl;
}

bool FileTreater::canRefresh() const
{
    return d->canRefreshFlag;
}

void FileTreater::refresh()
{
    if (!d->canRefreshFlag)
        return;

    d->canRefreshFlag = false;
    d->refreshedFlag = false;
    if (!d->traversalThread.isNull()) {
        d->traversalThread->quit();
        d->traversalThread->wait();
        disconnect(d->traversalThread.data());
    }
    d->traversalThread.reset(new TraversalDirThread(d->rootUrl, QStringList(), QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System, QDirIterator::NoIteratorFlags));
    if (d->traversalThread.isNull()) {
        d->canRefreshFlag = true;
        d->refreshedFlag = true;
        return;
    }
    QObject::connect(d->traversalThread.data(), &TraversalDirThread::updateChildren, d.data(), &FileTreaterPrivate::doUpdateChildren, Qt::QueuedConnection);
    d->traversalThread->start();
}

bool FileTreater::isRefreshed() const
{
    return d->refreshedFlag;
}

bool FileTreater::enableSort() const
{
    return d->enableSort;
}

void FileTreater::setEnabledSort(const bool enabledSort)
{
    if (enabledSort == d->enableSort)
        return;

    d->enableSort = enabledSort;

    emit enableSortChanged(enabledSort);
}

bool FileTreater::sort()
{
    // todo(wangcl)
    return true;
}

Qt::SortOrder FileTreater::sortOrder() const
{
    return d->sortOrder;
}

void FileTreater::setSortOrder(const Qt::SortOrder order)
{
    if (order == d->sortOrder)
        return;

    d->sortOrder = order;
}

int FileTreater::sortRole() const
{
    return d->sortRole;
}

void FileTreater::setSortRole(const int role, const Qt::SortOrder order)
{
    if (role != d->sortRole)
        d->sortRole = role;
    if (order != d->sortOrder)
        d->sortOrder = order;
}

DSB_D_END_NAMESPACE
