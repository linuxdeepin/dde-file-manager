// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsfilewatcher.h"
#include "private/avfsfilewatcher_p.h"

using namespace dfmplugin_avfsbrowser;

AvfsFileWatcher::AvfsFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new AvfsFileWatcherPrivate(url, this))
{
}

AvfsFileWatcher::~AvfsFileWatcher()
{
}

AvfsFileWatcherPrivate::AvfsFileWatcherPrivate(const QUrl &url, AvfsFileWatcher *qq)
    : AbstractFileWatcherPrivate(url, qq)
{
}
