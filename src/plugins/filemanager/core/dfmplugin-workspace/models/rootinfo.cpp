/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "rootinfo.h"
#include "fileitemdata.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"
#include "dpf.h"

using namespace dfmbase;
using namespace dfmplugin_workspace;

RootInfo::RootInfo(int i, const QUrl &u, const AbstractFileWatcherPointer &w)
    : rowIndex(i),
      canFetchMore(false),
      needTraversal(true),
      url(u),
      watcher(w),
      data(new FileItemData(u))
{
}

void RootInfo::init()
{
    startWatcher();

    canFetchMore = true;
}

QList<QUrl> RootInfo::getChildrenUrls() const
{
    if (fileCache) {
        return fileCache->childrenUrlList;
    }

    return {};
}

void RootInfo::clearChildren()
{
    if (fileCache)
        fileCache->clearChildren();
}

void RootInfo::startWatcher()
{
    if (!watcher.isNull()) {
        connect(watcher.data(), &AbstractFileWatcher::fileDeleted,
                this, &RootInfo::doFileDeleted);
        connect(watcher.data(), &AbstractFileWatcher::subfileCreated,
                this, &RootInfo::dofileCreated);
        connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                this, &RootInfo::doFileUpdated);
        connect(watcher.data(), &AbstractFileWatcher::fileRename,
                this, &RootInfo::dofileMoved);

        watcher->startWatcher();
    }
}

void RootInfo::refreshChildren()
{
    if (fileCache)
        fileCache->refreshChildren();
}

void RootInfo::newFileAdded(const QUrl &url)
{
    // TODO(liuyangming): do select new file
    //    if (WorkspaceHelper::kSelectionAndRenameFile.contains(windowId)) {
    //        if (WorkspaceHelper::kSelectionAndRenameFile[windowId].first == UrlRoute::urlParent(fileUrl)) {
    //            WorkspaceHelper::kSelectionAndRenameFile[windowId] = qMakePair(QUrl(), QUrl());

    //            QTimer::singleShot(100, this, [=] {
    //                emit selectAndEditFile(fileUrl);
    //            });
    //        }
    //    }
}

int RootInfo::childIndex(const QUrl &url) const
{
    if (fileCache)
        return fileCache->childrenUrlList.indexOf(url);

    return -1;
}

int RootInfo::childrenCount()
{
    if (fileCache)
        return fileCache->childrenCount();

    return 0;
}

void RootInfo::doFileDeleted(const QUrl &url)
{
    enqueueEvent(QPair<QUrl, EventType>(url, kRmFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::dofileMoved(const QUrl &fromUrl, const QUrl &toUrl)
{
    doFileDeleted(fromUrl);

    AbstractFileInfoPointer info = InfoCache::instance().getCacheInfo(toUrl);
    if (info)
        info->refresh();

    if (!fileCache->containsChild(toUrl))
        dofileCreated(toUrl);
}

void RootInfo::dofileCreated(const QUrl &url)
{
    enqueueEvent(QPair<QUrl, EventType>(url, kAddFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::doFileUpdated(const QUrl &url)
{
    AbstractFileInfoPointer info = InfoCache::instance().getCacheInfo(url);
    if (info)
        info->refresh();
}

void RootInfo::doWatcherEvent()
{
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

        if (UniversalUtils::urlEquals(fileUrl, url)) {
            if (event.second == kAddFile)
                continue;
            else if (event.second == kRmFile) {
                fileCache->clearChildren();
                needTraversal = true;
                dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Close", fileUrl);
                break;
            }
        }

        if (event.second == kAddFile) {
            fileCache->addChildren({ fileUrl });
            newFileAdded(fileUrl);
        } else {
            fileCache->removeChildren({ fileUrl });
            dpfSlotChannel->push("dfmplugin_workspace", "slot_Tab_Close", fileUrl);
        }
    }

    Q_EMIT childrenUpdate(url);
    processFileEventRuning = false;
}

bool RootInfo::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    return !watcherEvent.isEmpty();
}

void RootInfo::enqueueEvent(const QPair<QUrl, EventType> &e)
{
    QMutexLocker lk(&watcherEventMutex);
    watcherEvent.enqueue(e);
}

QPair<QUrl, RootInfo::EventType> RootInfo::dequeueEvent()
{
    QMutexLocker lk(&watcherEventMutex);
    return watcherEvent.dequeue();
}
