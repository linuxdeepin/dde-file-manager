// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VFSMONITORWATCHER_P_H
#define VFSMONITORWATCHER_P_H

#include "vfsmonitorwatcher.h"

#include <QAtomicInt>
#include <QMutex>
#include <QQueue>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QVector>

#include <cstdint>
#include <sys/types.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

// Userspace queue capacity. 500k events (~tens of MB worst case) comfortably
// covers a 300k-file burst; beyond that events are dropped and reported via
// eventsLost() so the service schedules a full update as the last fallback.
// Override for tests via DFM_VFSMONITOR_MAX_QUEUE.
inline constexpr int kDefaultMaxQueuedEvents = 500000;

class VfsSocketReader;

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

// One decoded dispatcher event parked in the userspace queue between the
// socket reader thread and the watcher's home thread.
struct QueuedFsEvent
{
    int action { 0 };
    uint32_t cookie { 0 };
    // Event path (absolute). For RENAME_TO events this stays empty; the
    // destination (resolved or not) is carried by pathB.
    QString pathA;
    // RENAME_TO only: resolved destination path, empty when the destination
    // lies outside the monitored roots (=> paired RENAME_FROM means deletion).
    QString pathB;
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

    // Tear down the current connection state and arm the reconnect timer.
    // Safe to call whether or not a connection exists. The socket and the
    // notifier are owned by the reader thread; only scheduling happens here.
    void handleDisconnect();
    // Timer callback: open a fresh dispatcher connection and hand it to the
    // reader thread; on failure grow the backoff.
    void attemptReconnect();
    // Create the socket, connect and enlarge the receive buffer. Returns the
    // fd on success, -1 on failure. Runs on the home thread; ownership of the
    // returned fd is transferred to the reader thread via startReaderThread /
    // attemptReconnect.
    int connectDispatcherSocket();
    // Spawn the reader thread and hand it a connected fd.
    void startReaderThread(int fd);

    // Reader-thread-safe: request an asynchronous drainQueuedEvents() on the
    // home thread. Coalesced via drainScheduled so bursts enqueue one wakeup.
    void scheduleEventDrain();
    // Home thread: dispatch up to a bounded batch of queued events as signals.
    void drainQueuedEvents();
    // Home thread: translate one queued event into the public signals.
    void dispatchQueuedEvent(const QueuedFsEvent &event);
    // The event dispatcher sends absolute paths, but they may use a different
    // mount alias than the monitored root path. This helper normalizes across
    // same-device mount aliases before applying rootPaths and excludePredicate.
    QString resolveAndFilterFullPath(const char *absolutePath) const;

    static QPair<QString, QString> splitPath(const QString &fullPath);

    VfsMonitorFileSystemWatcher *q_ptr;

    QStringList rootPaths;
    VfsMonitorFileSystemWatcher::PathExcludePredicate excludePredicate;

    // Reconnection state. After a disconnect the watcher keeps trying to
    // reconnect with exponential backoff so monitoring always recovers.
    QTimer *reconnectTimer { nullptr };
    int reconnectBackoffMs { 0 };

    // Reader thread. It owns the connected socket and its QSocketNotifier and
    // does nothing but drain the socket into eventQueue, so a slow consumer
    // can never make the dispatcher kick us as a "slow client".
    VfsSocketReader *reader { nullptr };
    QThread *readerThread { nullptr };
    // fd handed to the reader thread but not yet adopted (close in destructor
    // if the hand-off never completes).
    QAtomicInt pendingFd { -1 };

    // Userspace event queue between the reader thread and the home thread.
    // Guarded by queueMutex. If it fills up, events are dropped and
    // overflowFlag is set; the next drain reports VfsMonitor eventsLost().
    QMutex queueMutex;
    QQueue<QueuedFsEvent> eventQueue;
    QAtomicInt drainScheduled { 0 };
    QAtomicInt overflowFlag { 0 };
    int maxQueuedEvents { kDefaultMaxQueuedEvents };

    // Overridable socket path (env: DFM_VFSMONITOR_SOCKET_PATH). Defaults to
    // kDispatcherSocketPath; used by unit tests to point at a mock dispatcher.
    QString socketPath;

    QHash<uint32_t, RenameFromInfo> pendingRenames;
    QHash<dev_t, QStringList> mountPoints;
    QVector<MountPointAlias> orderedMountPoints;
    QVector<QPair<QString, QString>> rootAliases;

    bool initMountPoints();
    void rebuildRootAliases();
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // VFSMONITORWATCHER_P_H
