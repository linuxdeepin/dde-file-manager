// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VFSMONITORWATCHER_H
#define VFSMONITORWATCHER_H

#include "service_textindex_global.h"

#include <QObject>
#include <QStringList>
#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class VfsMonitorFileSystemWatcherPrivate;

// VfsMonitorFileSystemWatcher: File system watcher using deepin-anything
// event dispatcher socket exposed by deepin-anything-server.
//
// Receives global file system events (create, delete, move, close-write)
// forwarded by deepin-anything-server from
// /run/deepin-anything/event-dispatcher.sock.
//
// Usage:
//   auto *watcher = VfsMonitorFileSystemWatcher::create(rootPaths, excludePredicate, parent);
//   if (watcher) { ... }  // event dispatcher available
//   // else: fallback to inotify-only mode
class VfsMonitorFileSystemWatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(VfsMonitorFileSystemWatcher)
    Q_DECLARE_PRIVATE(VfsMonitorFileSystemWatcher)

public:
    // Predicate for path exclusion. Return true to suppress the event.
    // Called from the socket reader thread (not the home thread); it must
    // only touch thread-safe state (syscalls, atomics, const data).
    using PathExcludePredicate = std::function<bool(const QString &fullPath)>;

    ~VfsMonitorFileSystemWatcher() override;

    // Factory method: attempts to connect to the deepin-anything event
    // dispatcher socket. Returns nullptr if the dispatcher is not available
    // (graceful degradation).
    static VfsMonitorFileSystemWatcher *create(const QStringList &rootPaths,
                                               PathExcludePredicate excludePredicate,
                                               QObject *parent = nullptr);

Q_SIGNALS:
    void fileCreated(const QString &path, const QString &name);
    void fileDeleted(const QString &path, const QString &name);
    void fileMoved(const QString &fromPath, const QString &fromName,
                   const QString &toPath, const QString &toName);
    void directoryCreated(const QString &path, const QString &name);
    void directoryDeleted(const QString &path, const QString &name);
    void directoryMoved(const QString &fromPath, const QString &fromName,
                        const QString &toPath, const QString &toName);
    void fileClosed(const QString &path, const QString &name);

    // Emitted when filesystem events were lost and cannot be replayed:
    // either the dispatcher connection dropped (events during the outage)
    // or the internal event queue overflowed under an extreme burst.
    // Consumers should schedule a full index update to re-sync.
    void eventsLost();

private:
    explicit VfsMonitorFileSystemWatcher(const QStringList &rootPaths,
                                         PathExcludePredicate excludePredicate,
                                         QObject *parent = nullptr);

    QScopedPointer<VfsMonitorFileSystemWatcherPrivate> d_ptr;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // VFSMONITORWATCHER_H
