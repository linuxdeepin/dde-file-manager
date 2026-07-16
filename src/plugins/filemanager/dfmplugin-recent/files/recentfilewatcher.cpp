// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfilewatcher.h"
#include "private/recentfilewatcher_p.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;

RecentFileWatcherPrivate::RecentFileWatcherPrivate(const QUrl &fileUrl, RecentFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool RecentFileWatcherPrivate::start()
{
    started = true;
    return true;
}

bool RecentFileWatcherPrivate::stop()
{
    started = false;
    return true;
}

RecentFileWatcher::RecentFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new RecentFileWatcherPrivate(url, this), parent)
{
    dptr = static_cast<RecentFileWatcherPrivate *>(d.data());
}

RecentFileWatcher::~RecentFileWatcher()
{
}
