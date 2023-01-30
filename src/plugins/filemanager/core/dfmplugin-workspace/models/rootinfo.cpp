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

#include <dfm-framework/event/event.h>

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
    QString localFilePath = this->url.path();
    if (!data->fileInfo().isNull())
        localFilePath = data->fileInfo()->pathOf(PathInfoType::kFilePath);
    hiddenFileUrl = QUrl::fromLocalFile(localFilePath + "/.hidden");
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

    AbstractFileInfoPointer info = InfoCacheController::instance().getCacheInfo(toUrl);
    if (info)
        info->refresh();

    if (toUrl.scheme() != url.scheme())
        return;

    if (!fileCache->containsChild(toUrl)) {
        {
            // Before the file moved signal is received, `toUrl` may be filtered if it received a created signal
            QMutexLocker lk(&watcherEventMutex);
            auto iter = std::find_if(watcherEvent.cbegin(), watcherEvent.cend(), [&](const QPair<QUrl, EventType> &event) {
                return (UniversalUtils::urlEquals(toUrl, event.first) && event.second == kAddFile);
            });
            if (iter != watcherEvent.cend())
                watcherEvent.removeOne(*iter);
        }
        dofileCreated(toUrl);
    } else {
        // if watcherEvent exist toUrl rmevent,cancel this rmevent, beacuse if watcherEvent has toUrl rmevent will bypass this check
        QMutexLocker lk(&watcherEventMutex);
        auto iter = std::find_if(watcherEvent.cbegin(), watcherEvent.cend(), [&](const QPair<QUrl, EventType> &event) {
            return (UniversalUtils::urlEquals(toUrl, event.first) && event.second == kRmFile);
        });
        if (iter != watcherEvent.cend())
            watcherEvent.removeOne(*iter);
    }

    // TODO(lanxs) TODO(xust) .hidden file's attribute changed signal not emitted in removable disks (vfat/exfat).
    // but renamed from a .goutputstream_xxx file
    // NOTE: GlobalEventType::kHideFiles event is watched in fileview, but this can be used to notify update view
    // when the file is modified in other way.
    if (UniversalUtils::urlEquals(hiddenFileUrl, toUrl))
        Q_EMIT reloadView();
}

void RootInfo::dofileCreated(const QUrl &url)
{
    enqueueEvent(QPair<QUrl, EventType>(url, kAddFile));
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void RootInfo::doFileUpdated(const QUrl &url)
{
    AbstractFileInfoPointer info = InfoCacheController::instance().getCacheInfo(url);
    if (info)
        info->refresh();

    // NOTE: GlobalEventType::kHideFiles event is watched in fileview, but this can be used to notify update view
    // when the file is modified in other way.
    if (UniversalUtils::urlEquals(hiddenFileUrl, url))
        Q_EMIT reloadView();
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
            Q_EMIT selectAndEditFile(url, fileUrl);
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
