// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagfilewatcher_p.h"
#include "files/tagfilewatcher.h"
#include "utils/tagmanager.h"

#include <dfm-base/base/schemefactory.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_tag;

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
        fmWarning("watcher create failed.");
        abort();
    }
}

void TagFileWatcherPrivate::initConnect()
{
    connect(proxy.data(), &AbstractFileWatcher::fileDeleted, q, &AbstractFileWatcher::fileDeleted);
    connect(proxy.data(), &AbstractFileWatcher::fileAttributeChanged, q, &AbstractFileWatcher::fileAttributeChanged);
    connect(proxy.data(), &AbstractFileWatcher::subfileCreated, q, &AbstractFileWatcher::subfileCreated);

    connect(TagManager::instance(), &TagManager::tagDeleted, qobject_cast<TagFileWatcher *>(q), &TagFileWatcher::onTagRemoved);
    connect(TagManager::instance(), &TagManager::filesTagged, qobject_cast<TagFileWatcher *>(q), &TagFileWatcher::onFilesTagged);
    connect(TagManager::instance(), &TagManager::filesUntagged, qobject_cast<TagFileWatcher *>(q), &TagFileWatcher::onFilesUntagged);
    connect(TagManager::instance(), &TagManager::filesHidden, qobject_cast<TagFileWatcher *>(q), &TagFileWatcher::onFilesHidden);
}
