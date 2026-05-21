// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VFSMONITORWATCHER_P_H
#define VFSMONITORWATCHER_P_H

#include "vfsmonitorwatcher.h"

#include <QSocketNotifier>
#include <QHash>
#include <QSet>
#include <QStringList>

#include <sys/types.h>

struct nl_sock;

SERVICETEXTINDEX_BEGIN_NAMESPACE

// Stores information from a RENAME_FROM event, awaiting RENAME_TO pairing.
struct RenameFromInfo
{
    QString path;
    QString name;
    bool isDirectory { false };
};

// Netlink attribute indices (matching vfs_genl.h)
enum VfsMonitorAttr : int {
    VFSMONITOR_A_UNSPEC = 0,
    VFSMONITOR_A_ACT,
    VFSMONITOR_A_COOKIE,
    VFSMONITOR_A_MAJOR,
    VFSMONITOR_A_MINOR,
    VFSMONITOR_A_PATH,
    VFSMONITOR_A_UID,
    VFSMONITOR_A_TGID,
    __VFSMONITOR_A_MAX
};
#define VFSMONITOR_A_MAX (__VFSMONITOR_A_MAX - 1)

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

    bool initNetlink();
    void handleNetlinkMessage();

    // Reconstruct full path from relative path + device, then check against
    // rootPaths and excludePredicate. Returns null if filtered out.
    QString resolveAndFilterFullPath(dev_t deviceId, const char *relativePath) const;

    static QPair<QString, QString> splitPath(const QString &fullPath);

    VfsMonitorFileSystemWatcher *q_ptr;

    QStringList rootPaths;
    VfsMonitorFileSystemWatcher::PathExcludePredicate excludePredicate;

    nl_sock *nlSock { nullptr };
    QSocketNotifier *notifier { nullptr };

    QHash<uint32_t, RenameFromInfo> pendingRenames;

    // dev_t -> sorted mount point list (longest first).
    QHash<dev_t, QStringList> mountPoints;
    QSet<dev_t> devicesWithChildren;

    bool initMountPoints();
    bool hasChildMountPoints(dev_t deviceId) const;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // VFSMONITORWATCHER_P_H
