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
// vfs_monitor kernel module via Linux Generic Netlink multicast.
//
// Receives global file system events (create, delete, move) from the kernel,
// covering all mounted filesystems without the need for recursive inotify watches.
// File close events (IN_CLOSE_WRITE) are NOT provided -- use InotifyFileSystemWatcher
// for those.
//
// Usage:
//   auto *watcher = VfsMonitorFileSystemWatcher::create(rootPaths, excludePredicate, parent);
//   if (watcher) { ... }  // kernel module available
//   // else: fallback to inotify-only mode
class VfsMonitorFileSystemWatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(VfsMonitorFileSystemWatcher)
    Q_DECLARE_PRIVATE(VfsMonitorFileSystemWatcher)

public:
    // Predicate for path exclusion. Return true to suppress the event.
    // Called from the netlink callback thread context (main thread via Qt event loop).
    using PathExcludePredicate = std::function<bool(const QString &fullPath)>;

    ~VfsMonitorFileSystemWatcher() override;

    // Factory method: attempts to connect to the vfsmonitor Generic Netlink family.
    // Returns nullptr if the kernel module is not available (graceful degradation).
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

private:
    explicit VfsMonitorFileSystemWatcher(const QStringList &rootPaths,
                                         PathExcludePredicate excludePredicate,
                                         QObject *parent = nullptr);

    QScopedPointer<VfsMonitorFileSystemWatcherPrivate> d_ptr;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // VFSMONITORWATCHER_H
