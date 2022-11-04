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
#include "filedatahelper.h"
#include "models/fileviewmodel.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/urlroute.h"

using namespace dfmbase;
using namespace dfmbase::Global;
using namespace dfmplugin_workspace;

RootInfo *FileDataHelper::setRoot(const QUrl &rootUrl)
{
    if (rootInfoMap.contains(rootUrl)) {
        if (rootUrl.scheme() == Scheme::kFile && !rootInfoMap[rootUrl]->needTraversal)
            return rootInfoMap[rootUrl];

        RootInfo *info = rootInfoMap[rootUrl];
        info->clearChildren();
        info->needTraversal = true;

        const AbstractFileWatcherPointer &watcher = setWatcher(rootUrl);
        info->watcher = watcher;
        info->startWatcher();

        return info;
    }

    RootInfo *info = createRootInfo(rootUrl);
    rootInfoMap[rootUrl] = info;

    info->startWatcher();

    return rootInfoMap[rootUrl];
}

RootInfo *FileDataHelper::findRootInfo(const QUrl &url)
{
    if (rootInfoMap.contains(url))
        return rootInfoMap[url];

    return nullptr;
}

RootInfo *FileDataHelper::findRootInfo(const int rowIndex)
{
    auto iter = rootInfoMap.begin();
    for (; iter != rootInfoMap.end(); ++iter) {
        if (rowIndex == iter.value()->rowIndex)
            return iter.value();
    }

    return nullptr;
}

FileItemData *FileDataHelper::findFileItemData(const int rootIndex, const int childIndex)
{
    auto root = findRootInfo(rootIndex);
    if (root && root->childrenCount() > childIndex)
        return root->fileCache->getChild(childIndex);

    return nullptr;
}

QPair<int, int> FileDataHelper::getIndexByUrl(const QUrl &url)
{
    if (!url.isValid())
        return QPair<int, int>(-1, -1);

    if (rootInfoMap.contains(url))
        return QPair<int, int>(rootInfoMap[url]->rowIndex, -1);

    const QUrl &parentUrl = UrlRoute::urlParent(url);
    if (rootInfoMap.contains(parentUrl))
        return QPair<int, int>(rootInfoMap[parentUrl]->rowIndex, rootInfoMap[parentUrl]->childIndex(url));

    return QPair<int, int>(-1, -1);
}

int FileDataHelper::rootsCount()
{
    return rootInfoMap.count();
}

int FileDataHelper::filesCount(const int rootIndex)
{
    auto info = findRootInfo(rootIndex);
    if (info)
        return info->childrenCount();

    return 0;
}

void FileDataHelper::doTravers(const int rootIndex)
{
    auto info = findRootInfo(rootIndex);

    if (info && info->needTraversal) {
        info->canFetchMore = false;
        info->needTraversal = false;

        info->traversal = setTraversalThread(info);

        if (!info->fileCache.isNull()) {
            info->fileCache->stop();
            info->fileCache->disconnect();
            info->fileCache->deleteLater();
        }

        info->fileCache.reset(new FileDataCacheThread(info));

        connect(info->traversal.data(), &TraversalDirThread::updateChild,
                info->fileCache.data(), &FileDataCacheThread::onHandleAddFile,
                Qt::QueuedConnection);
        connect(info->traversal.data(), &TraversalDirThread::updateChildren,
                this, [this, info](const QList<QUrl> &urls) {
                    if (urls.isEmpty())
                        this->model()->stateChanged(info->url, ModelState::kIdle);
                });
        connect(info->traversal.data(), &QThread::finished,
                info->fileCache.data(), &FileDataCacheThread::onHandleTraversalFinished,
                Qt::QueuedConnection);
        connect(info->fileCache.data(), &FileDataCacheThread::requestSetIdle,
                this, [this, info] {
                    this->model()->stateChanged(info->url, ModelState::kIdle);
                },
                Qt::QueuedConnection);

        info->traversal->start();
    }
}

void FileDataHelper::doStopWork(const QUrl &rootUrl)
{
    RootInfo *info = findRootInfo(rootUrl);
    if (info) {

        if (!info->traversal.isNull() && info->traversal->isRunning()) {
            info->traversal->stop();
            info->needTraversal = true;
        }

        if (!info->fileCache.isNull() && info->fileCache->isRunning())
            info->fileCache->stop();
    }
}

void FileDataHelper::update(const QUrl &rootUrl)
{
    if (rootInfoMap.contains(rootUrl))
        rootInfoMap[rootUrl]->refreshChildren();
}

void FileDataHelper::clear(const QUrl &rootUrl)
{
    RootInfo *info = findRootInfo(rootUrl);
    if (info)
        info->clearChildren();
}

void FileDataHelper::setFileActive(const int rootIndex, const int childIndex, bool active)
{
    RootInfo *info = findRootInfo(rootIndex);
    if (info) {
        FileItemData *data = info->fileCache->getChild(childIndex);
        if (data && data->fileInfo() && info->watcher)
            info->watcher->setEnabledSubfileWatcher(data->fileInfo()->url(), active);
    }
}

FileViewModel *FileDataHelper::model() const
{
    return dynamic_cast<FileViewModel *>(parent());
}

RootInfo *FileDataHelper::createRootInfo(const QUrl &url)
{
    const AbstractFileWatcherPointer &watcher = setWatcher(url);

    int index = rootInfoMap.count();

    RootInfo *info = new RootInfo(index, url, watcher);

    return info;
}

FileDataHelper::FileDataHelper(QObject *parent)
    : QObject(parent)
{
}

int FileDataHelper::preSetRoot(const QUrl &rootUrl)
{
    if (rootInfoMap.contains(rootUrl))
        return rootInfoMap[rootUrl]->rowIndex;

    return rootInfoMap.count();
}

AbstractFileWatcherPointer FileDataHelper::setWatcher(const QUrl &url)
{
    if (rootInfoMap.count(url) && !rootInfoMap[url]->watcher.isNull()) {
        const AbstractFileWatcherPointer &watcher = rootInfoMap[url]->watcher;

        watcher->disconnect();
        watcher->stopWatcher();
    }

    const AbstractFileWatcherPointer &watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (watcher.isNull()) {
        qWarning() << "Create watcher failed! url = " << url;
    }

    return watcher;
}

TraversalThreadPointer FileDataHelper::setTraversalThread(RootInfo *info)
{
    if (!info->traversal.isNull()) {
        const TraversalThreadPointer &traversal = info->traversal;

        traversal->disconnect();
        traversal->stopAndDeleteLater();
        traversal->setParent(nullptr);
    }

    TraversalThreadPointer traversal(new TraversalDirThread(
            info->url, QStringList(),
            QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
            QDirIterator::FollowSymlinks));

    return traversal;
}
