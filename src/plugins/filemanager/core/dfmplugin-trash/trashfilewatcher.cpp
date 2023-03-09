// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashfilewatcher.h"
#include "utils/trashhelper.h"
#include "private/trashfilewatcher_p.h"
#include "dfm-base/base/schemefactory.h"

#include "base/urlroute.h"

#include <dfm-io/dwatcher.h>

#include <QEvent>
#include <QDir>
#include <QDebug>
#include <QApplication>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_trash {

TrashFileWatcherPrivate::TrashFileWatcherPrivate(const QUrl &fileUrl, TrashFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool TrashFileWatcherPrivate::start()
{
    if (watcher.isNull())
        return false;
    started = watcher->start();
    if (!started)
        qWarning() << "watcher start failed, error: " << watcher->lastError().errorMsg();
    return started;
}

bool TrashFileWatcherPrivate::stop()
{
    if (watcher.isNull())
        return false;
    started = watcher->stop();
    return started;
}

void TrashFileWatcherPrivate::initFileWatcher()
{
    const QUrl &trashUrl = url;
    watcher.reset(new DWatcher(trashUrl));
    if (!watcher) {
        qWarning("watcher create failed.");
        abort();
    }
}

void TrashFileWatcherPrivate::initConnect()
{
    connect(watcher.data(), &DWatcher::fileChanged, q, &AbstractFileWatcher::fileAttributeChanged);
    connect(watcher.data(), &DWatcher::fileDeleted, q, &AbstractFileWatcher::fileDeleted);
    connect(watcher.data(), &DWatcher::fileAdded, q, &AbstractFileWatcher::subfileCreated);
    connect(watcher.data(), &DWatcher::fileRenamed, q, &AbstractFileWatcher::fileRename);
}

TrashFileWatcher::TrashFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new TrashFileWatcherPrivate(url, this), parent)
{
    dptr = static_cast<TrashFileWatcherPrivate *>(d.data());
    dptr->initFileWatcher();
    dptr->initConnect();
}

TrashFileWatcher::~TrashFileWatcher()
{
}

}
