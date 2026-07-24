// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VFSMONITORWATCHER_P_H
#define VFSMONITORWATCHER_P_H

#include "vfsmonitorwatcher.h"

#include <QSocketNotifier>
#include <QHash>
#include <QStringList>

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
    ACT_UNMOUNT = 13
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

    QHash<uint32_t, RenameFromInfo> pendingRenames;
    QHash<dev_t, QStringList> mountPoints;

    bool initMountPoints();
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // VFSMONITORWATCHER_P_H
