// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashfilewatcher.h"
#include "utils/trashhelper.h"
#include "private/trashfilewatcher_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/trashutils.h>

#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

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
    if (watchers.isEmpty()) {
        fmWarning() << "Trash: Cannot start watcher, no watchers available";
        return false;
    }
    started = true;
    for (auto &w : watchers) {
        if (w && !w->startWatcher()) {
            fmWarning() << "Trash: Watcher start failed for local trash dir";
            started = false;
        }
    }
    return started;
}

bool TrashFileWatcherPrivate::stop()
{
    if (watchers.isEmpty()) {
        fmWarning() << "Trash: Cannot stop watcher, no watchers available";
        return false;
    }
    started = false;
    for (auto &w : watchers) {
        if (w)
            w->stopWatcher();
    }
    return true;
}

void TrashFileWatcherPrivate::initFileWatcher()
{
    // Create LocalFileWatcher on local trash directories instead of DWatcher
    // on trash:/// URL to avoid blocking through gvfsd on stale CIFS mounts.
    // LocalFileWatcher reuses the project's existing watcher infrastructure
    // and avoids wasting extra inotify instances.
    const auto &dirs = TrashUtils::localTrashDirs();
    for (const QString &dir : dirs) {
        if (!QDir(dir).exists())
            continue;
        QUrl localUrl = QUrl::fromLocalFile(dir);
        watchers.append(QSharedPointer<AbstractFileWatcher>(new LocalFileWatcher(localUrl, q)));
    }

    if (watchers.isEmpty()) {
        fmWarning() << "Trash: No local trash directories found for watching";
    }
}

void TrashFileWatcherPrivate::initConnect()
{
    for (auto &w : watchers) {
        if (!w)
            continue;
        connect(w.data(), &AbstractFileWatcher::fileAttributeChanged, q, [&](const QUrl &url) {
            emit q->fileAttributeChanged(FileUtils::bindUrlTransform(url));
        });
        connect(w.data(), &AbstractFileWatcher::fileDeleted, q, [&](const QUrl &url) {
            emit q->fileDeleted(FileUtils::bindUrlTransform(url));
        });
        connect(w.data(), &AbstractFileWatcher::subfileCreated, q, [&](const QUrl &url) {
            emit q->subfileCreated(FileUtils::bindUrlTransform(url));
        });
        connect(w.data(), &AbstractFileWatcher::fileRename, q, [&](const QUrl &from, const QUrl &to) {
            emit q->fileRename(FileUtils::bindUrlTransform(from), FileUtils::bindUrlTransform(to));
        });
    }
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
