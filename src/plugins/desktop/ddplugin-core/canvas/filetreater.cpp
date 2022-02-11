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
//#include "canvas/displayconfig.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/base/urlroute.h"

#include <QDir>
#include <QtConcurrent>
#include <QStandardPaths>

DSB_D_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class FileTreaterGlobal : public FileTreater
{
};
Q_GLOBAL_STATIC(FileTreaterGlobal, fileTreater)

FileTreaterPrivate::FileTreaterPrivate(FileTreater *q_ptr)
    : QObject(q_ptr), q(q_ptr)
{
}

bool FileTreaterPrivate::doSort(QList<DFMLocalFileInfoPointer> &files) const
{
    if (files.isEmpty())
        return true;

    auto firstInfo = files.first();
    AbstractFileInfo::CompareFunction sortFunc = firstInfo->compareFunByKey(sortRole);
    if (sortFunc) {
        // standard sort function
        std::sort(files.begin(), files.end(), [sortFunc, this](const DFMLocalFileInfoPointer info1, const DFMLocalFileInfoPointer info2) {
            return sortFunc(info1, info2, sortOrder);
        });

        // advanced sort for special case.
        specialSort(files);
        return true;
    } else {
        return false;
    }
}

void FileTreaterPrivate::doFileDeleted(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        QPair<EventType, QVariant> rmFile(kRmFile, QVariant(url));
        QVariant data = QVariant::fromValue(rmFile);
        watcherEvent.enqueue(data);
    }

    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileTreaterPrivate::doFileRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        QPair<QUrl, QUrl> urls(oldUrl, newUrl);
        QVariant varUrls = QVariant::fromValue(urls);
        QPair<EventType, QVariant> reFile(kReFile, varUrls);
        QVariant data = QVariant::fromValue(reFile);
        watcherEvent.enqueue(data);
    }

    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileTreaterPrivate::doFileCreated(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        QPair<EventType, QVariant> rmFile(kAddFile, QVariant(url));
        QVariant data = QVariant::fromValue(rmFile);
        watcherEvent.enqueue(data);
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
    QList<DFMLocalFileInfoPointer> files;
    QString errString;
    for (auto children : childrens) {
        auto itemInfo = dfmbase::InfoFactory::create<dfmbase::LocalFileInfo>(children, true, &errString);
        if (Q_UNLIKELY(!itemInfo)) {
            qInfo() << "create LocalFileInfo error: " << errString;
            continue;
        }

        files.append(itemInfo);
    }

    // defalut sort
    doSort(files);

    {
        QList<QUrl> fileUrls;
        QMap<QUrl, DFMLocalFileInfoPointer> fileMaps;
        for (auto itemInfo : files) {
            if (Q_UNLIKELY(!itemInfo))
                continue;

            fileUrls.append(itemInfo->url());
            fileMaps.insert(itemInfo->url(), itemInfo);
        }

        fileList = fileUrls;
        fileMap = fileMaps;
    }

    endRefresh();
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
        QVariant event;
        {
            QMutexLocker lk(&watcherEventMutex);
            event = watcherEvent.dequeue();
        }

        if (!event.canConvert<QPair<EventType, QVariant>>()) {
            qInfo() << "data error format:" << event;
            continue;
        }

        QPair<EventType, QVariant> eventData = event.value<QPair<EventType, QVariant>>();
        const EventType eventType = eventData.first;

        if (kAddFile == eventType) {
            const QUrl &url = eventData.second.toUrl();
            if (Q_UNLIKELY(fileList.contains(url)))
                continue;

            QString errString;
            auto itemInfo = dfmbase::InfoFactory::create<dfmbase::LocalFileInfo>(url, true, &errString);
            if (Q_UNLIKELY(!itemInfo)) {
                qInfo() << "create LocalFileInfo error: " << errString << url;
                continue;
            }

            fileList.append(url);
            fileMap.insert(url, itemInfo);

            emit q->fileCreated(url);
        } else if (kRmFile == eventType) {
            const QUrl &url = eventData.second.toUrl();
            if (Q_UNLIKELY(!fileList.contains(url)))
                continue;
            fileList.removeOne(url);
            fileMap.remove(url);

            emit q->fileDeleted(url);
        } else if (kReFile == eventType) {
            const QPair<QUrl, QUrl> urls = eventData.second.value<QPair<QUrl, QUrl>>();
            const QUrl &oldUrl = urls.first;
            const QUrl &newUrl = urls.second;

            if (Q_UNLIKELY(!fileList.contains(oldUrl)) || Q_UNLIKELY(fileList.contains(newUrl))) {
                qWarning() << "unknow error in rename file:" << fileList.contains(oldUrl) << oldUrl << fileList.contains(newUrl) << newUrl;
            }

            int index = fileList.indexOf(oldUrl);
            if (Q_LIKELY(-1 != index)) {
                fileList.removeAt(index);
            }
            fileMap.remove(oldUrl);

            QString errString;
            auto info = dfmbase::InfoFactory::create<dfmbase::LocalFileInfo>(newUrl, true, &errString);
            if (Q_UNLIKELY(!info)) {
                qInfo() << "create LocalFileInfo error: " << errString << newUrl;
                continue;
            }

            if (Q_LIKELY(-1 != index)) {
                fileList.insert(index, newUrl);
            } else {
                fileList.append(newUrl);
            }
            fileMap.insert(newUrl, info);

            emit q->fileRenamed(oldUrl, newUrl);
        }
    }
    processFileEventRuning = false;
}

void FileTreaterPrivate::beginRefresh()
{
    canRefreshFlag = false;
    refreshedFlag = false;
}

void FileTreaterPrivate::endRefresh()
{
    canRefreshFlag = true;
    refreshedFlag = true;
}

void FileTreaterPrivate::specialSort(QList<DFMLocalFileInfoPointer> &files) const
{
    if (sortRole == AbstractFileInfo::kSortByFileMimeType)
        sortMainDesktopFile(files, sortOrder);
}

void FileTreaterPrivate::sortMainDesktopFile(QList<DFMLocalFileInfoPointer> &files, Qt::SortOrder order) const
{
    // let the main desktop files always on front or back.

    //! warrning: the root url and LocalFileInfo::url must be like file://
    QDir dir(rootUrl.toString());
    QList<QPair<QString, DFMLocalFileInfoPointer>> mainDesktop = {{dir.filePath("dde-home.desktop"), DFMLocalFileInfoPointer()},
        {dir.filePath("dde-trash.desktop"), DFMLocalFileInfoPointer()},
        {dir.filePath("dde-computer.desktop"), DFMLocalFileInfoPointer()}
    };
    auto list = files;
    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        for (const DFMLocalFileInfoPointer &info : list)
        if (info->url().toString() == it->first) {
            it->second = info;
            files.removeOne(info);
        }
    }

    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        if (it->second) {
            if (order == Qt::AscendingOrder)
                files.push_front(it->second);
            else
                files.push_back(it->second);
        }
    }
}

bool FileTreaterPrivate::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    bool isEmptyQueue = watcherEvent.isEmpty();
    return !isEmptyQueue;
}

FileTreater::FileTreater(QObject *parent)
    : QObject(parent), d(new FileTreaterPrivate(this))
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
    if (Q_UNLIKELY(!d->rootUrl.isValid())) {
        qWarning() << "desktop url is invalid:" << d->rootUrl;
        return;
    }

    if (!d->watcher.isNull()) {
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileDeleted, d.data(), &FileTreaterPrivate::doFileDeleted);
        disconnect(d->watcher.data(), &AbstractFileWatcher::subfileCreated, d.data(), &FileTreaterPrivate::doFileCreated);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileRename, d.data(), &FileTreaterPrivate::doFileRename);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged, d.data(), &FileTreaterPrivate::doFileUpdated);
    }

    d->watcher = WacherFactory::create<AbstractFileWatcher>(d->rootUrl);
    if (Q_LIKELY(!d->watcher.isNull())) {
        connect(d->watcher.data(), &AbstractFileWatcher::fileDeleted, d.data(), &FileTreaterPrivate::doFileDeleted);
        connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated, d.data(), &FileTreaterPrivate::doFileCreated);
        connect(d->watcher.data(), &AbstractFileWatcher::fileRename, d.data(), &FileTreaterPrivate::doFileRename);
        connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged, d.data(), &FileTreaterPrivate::doFileUpdated);
        d->watcher->startWatcher();
    }

    d->filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;

    // todo(wangcl):by DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedHiddenFiles).toBool()
    d->whetherShowHiddenFile = false;
    if (d->whetherShowHiddenFile)
        d->filters |= QDir::Hidden;
    else
        d->filters &= ~QDir::Hidden;

    d->refreshedFlag = false;
    d->canRefreshFlag = true;
}

QList<QUrl> FileTreater::files() const
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
    if (!canRefresh())
        return;

    d->beginRefresh();
    if (!d->traversalThread.isNull()) {
        d->traversalThread->quit();
        d->traversalThread->wait();
        disconnect(d->traversalThread.data());
    }

    d->traversalThread.reset(new TraversalDirThread(d->rootUrl, QStringList(), d->filters, QDirIterator::NoIteratorFlags));
    if (Q_UNLIKELY(d->traversalThread.isNull())) {
        d->endRefresh();
        return;
    }

    QObject::connect(d->traversalThread.data(), &TraversalDirThread::updateChildren, d.data(), &FileTreaterPrivate::doUpdateChildren, Qt::QueuedConnection);
    d->traversalThread->start();
}

bool FileTreater::isRefreshed() const
{
    return d->refreshedFlag;
}

bool FileTreater::sort()
{
    if (d->fileList.isEmpty())
        return true;

    d->beginRefresh();

    auto files = d->fileMap.values();
    if (!d->doSort(files)) {
        d->endRefresh();
        return false;
    }

    QList<QUrl> fileList;
    QMap<QUrl, DFMLocalFileInfoPointer> fileMap;
    for (auto itemInfo : files) {
        if (Q_UNLIKELY(!itemInfo))
            continue;

        fileList.append(itemInfo->url());
        fileMap.insert(itemInfo->url(), itemInfo);
    }

    d->fileList = fileList;
    d->fileMap = fileMap;

    d->endRefresh();
    emit fileSorted();

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

void FileTreater::setSortRole(const AbstractFileInfo::SortKey role, const Qt::SortOrder order)
{
    if (role != d->sortRole)
        d->sortRole = role;
    if (order != d->sortOrder)
        d->sortOrder = order;
}

bool FileTreater::whetherShowHiddenFiles() const
{
    return d->whetherShowHiddenFile;
}

void FileTreater::setWhetherShowHiddenFiles(const bool isShow)
{
    if (d->whetherShowHiddenFile == isShow)
        return;
    d->whetherShowHiddenFile = isShow;
    if (d->whetherShowHiddenFile)
        d->filters |= QDir::Hidden;
    else
        d->filters &= ~QDir::Hidden;
}

DSB_D_END_NAMESPACE
