// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VFSMONITORWATCHER_P_H
#define VFSMONITORWATCHER_P_H

#include "vfsmonitorwatcher.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QHash>
#include <QStringList>
#include <QVector>

#include <cstdint>
#include <sys/types.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

// Stores information from a RENAME_FROM event, awaiting RENAME_TO pairing.
struct RenameFromInfo
{
    QString path;
    QString name;
    bool isDirectory { false };
};

struct MountPointAlias
{
    dev_t deviceId { 0 };
    QString mountPoint;
};

// Event action constants (matching vfs_change_consts.h)
enum VfsMonitorAct : uint8_t {
    ACT_NEW_FILE = 0,
    ACT_NEW_LINK = 1,
    ACT_NEW_SYMLINK = 2,
    ACT_NEW_FOLDER = 3,
    ACT_DEL_FILE = 4,
    ACT_DEL_FOLDER = 5,
    ACT_RENAME_FILE = 6,
    ACT_RENAME_FOLDER = 7,
    ACT_RENAME_FROM_FILE = 8,
    ACT_RENAME_TO_FILE = 9,
    ACT_RENAME_FROM_FOLDER = 10,
    ACT_RENAME_TO_FOLDER = 11,
    ACT_MOUNT = 12,
    ACT_UNMOUNT = 13,
    ACT_CLOSE_WRITE_FILE = 14
};

class VfsMonitorFileSystemWatcherPrivate
{
    Q_DECLARE_PUBLIC(VfsMonitorFileSystemWatcher)

public:
    VfsMonitorFileSystemWatcherPrivate(const QStringList &rootPaths,
                                       VfsMonitorFileSystemWatcher::PathExcludePredicate excludePredicate,
                                       VfsMonitorFileSystemWatcher *qq);
    ~VfsMonitorFileSystemWatcherPrivate();

    bool initDispatcher();
    void handleSocketMessage();

    // Tear down the current connection (socket + notifier) and arm the
    // reconnect timer. Safe to call whether or not a connection exists.
    void handleDisconnect();
    // Timer callback: attempt a fresh socket()+connect(); on success rebuild
    // the notifier and stop the timer; on failure grow the backoff.
    void attemptReconnect();
    // Create the socket, connect, enlarge the receive buffer and build the
    // notifier. Returns true on success. Does not touch the reconnect timer.
    bool establishConnection();

    // The event dispatcher sends absolute paths, but they may use a different
    // mount alias than the monitored root path. This helper normalizes across
    // same-device mount aliases before applying rootPaths and excludePredicate.
    QString resolveAndFilterFullPath(const char *absolutePath) const;

    static QPair<QString, QString> splitPath(const QString &fullPath);

    VfsMonitorFileSystemWatcher *q_ptr;

    QStringList rootPaths;
    VfsMonitorFileSystemWatcher::PathExcludePredicate excludePredicate;

    int socketFd { -1 };
    QSocketNotifier *notifier { nullptr };

    // Reconnection state. After a disconnect the watcher keeps trying to
    // reconnect with exponential backoff so monitoring always recovers.
    QTimer *reconnectTimer { nullptr };
    int reconnectBackoffMs { 0 };

    // Overridable socket path (env: DFM_VFSMONITOR_SOCKET_PATH). Defaults to
    // kDispatcherSocketPath; used by unit tests to point at a mock dispatcher.
    QString socketPath;

    QHash<uint32_t, RenameFromInfo> pendingRenames;
    QHash<dev_t, QStringList> mountPoints;
    QVector<MountPointAlias> orderedMountPoints;

    bool initMountPoints();
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // VFSMONITORWATCHER_P_H
