/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#include "tagfilewatcher_p.h"
#include "files/tagfilewatcher.h"
#include "utils/tagmanager.h"

#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPTAG_USE_NAMESPACE

TagFileWatcherPrivate::TagFileWatcherPrivate(const QUrl &fileUrl, TagFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool TagFileWatcherPrivate::start()
{
    return proxy && proxy->startWatcher();
}

bool TagFileWatcherPrivate::stop()
{
    return proxy && proxy->stopWatcher();
}

void TagFileWatcherPrivate::initFileWatcher()
{
    QUrl watchUrl = QUrl::fromLocalFile(path);

    proxy = WatcherFactory::create<AbstractFileWatcher>(watchUrl);

    if (!proxy) {
        qWarning("watcher create failed.");
        abort();
    }
}

void TagFileWatcherPrivate::initConnect()
{
    connect(proxy.data(), &AbstractFileWatcher::fileDeleted, q, &AbstractFileWatcher::fileDeleted);
    connect(proxy.data(), &AbstractFileWatcher::fileAttributeChanged, q, &AbstractFileWatcher::fileAttributeChanged);
    connect(proxy.data(), &AbstractFileWatcher::subfileCreated, q, &AbstractFileWatcher::subfileCreated);

    connect(TagManager::instance(), &TagManager::filesTagged, qobject_cast<TagFileWatcher *>(q), &TagFileWatcher::onFilesTagged);
    connect(TagManager::instance(), &TagManager::filesUntagged, qobject_cast<TagFileWatcher *>(q), &TagFileWatcher::onFilesUntagged);
}
