// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vfsmonitorwatcher_p.h"

#include <QDir>
#include <QFileInfo>

#include <libmount.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <algorithm>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

constexpr char kDispatcherSocketPath[] = "/run/deepin-anything/event-dispatcher.sock";
constexpr size_t kDispatchMaxPathLen = 4096;

struct MountEntry
{
    dev_t deviceId { 0 };
    int parentMountId { 0 };
    QString mountPoint;
    bool isBindMount { false };
};

struct DispatchEvent
{
    int32_t action;
    uint32_t cookie;
    char eventPath[kDispatchMaxPathLen];
};

bool isDescendantOfRoot(const QString &path, const QString &root)
{
    if (root == "/")
        return path.startsWith('/');

    if (!path.startsWith(root))
        return false;

    if (path.length() == root.length())
        return false;

    return root.endsWith('/') || path.at(root.length()) == '/';
}

bool mountPointStartsWith(const QString &path, const QString &mountPoint)
{
    if (!path.startsWith(mountPoint))
        return false;

    return mountPoint.endsWith('/') || path.length() == mountPoint.length()
            || path.at(mountPoint.length()) == '/';
}

bool cStringEquals(const char *left, const char *right)
{
    return left && right && qstrcmp(left, right) == 0;
}

bool isParentChainUnderRoot(const QHash<int, MountEntry> &byMountId, const MountEntry &entry)
{
    if (!entry.isBindMount && entry.mountPoint == "/")
        return true;

    int parentMountId = entry.parentMountId;
    while (parentMountId > 0) {
        auto parentIt = byMountId.find(parentMountId);
        if (parentIt == byMountId.end())
            return false;

        if (parentIt->mountPoint == "/")
            return true;

        parentMountId = parentIt->parentMountId;
    }

    return false;
}

QHash<int, MountEntry> collectMountEntries(libmnt_table *mtab)
{
    QHash<int, MountEntry> byMountId;
    libmnt_iter *iter = mnt_new_iter(MNT_ITER_FORWARD);
    if (!iter)
        return byMountId;

    libmnt_fs *fs = nullptr;
    while (mnt_table_next_fs(mtab, iter, &fs) == 0) {
        const char *target = mnt_fs_get_target(fs);
        if (!target)
            continue;

        MountEntry entry;
        entry.deviceId = mnt_fs_get_devno(fs);
        entry.parentMountId = mnt_fs_get_parent_id(fs);
        entry.mountPoint = QString::fromUtf8(target);
        entry.isBindMount = !cStringEquals(mnt_fs_get_root(fs), "/");

        byMountId.insert(mnt_fs_get_id(fs), entry);
    }

    mnt_free_iter(iter);
    return byMountId;
}

QString filterDirectPath(const QStringList &rootPaths,
                         const VfsMonitorFileSystemWatcher::PathExcludePredicate &excludePredicate,
                         const QString &fullPath)
{
    if (std::none_of(rootPaths.cbegin(), rootPaths.cend(),
                     [&fullPath](const QString &root) { return isDescendantOfRoot(fullPath, root); })) {
        return {};
    }

    if (excludePredicate && excludePredicate(fullPath))
        return {};

    return fullPath;
}

}   // anonymous namespace

// ========== VfsMonitorFileSystemWatcherPrivate ==========

VfsMonitorFileSystemWatcherPrivate::VfsMonitorFileSystemWatcherPrivate(
        const QStringList &rootPaths,
        VfsMonitorFileSystemWatcher::PathExcludePredicate excludePredicate,
        VfsMonitorFileSystemWatcher *qq)
    : q_ptr(qq), excludePredicate(std::move(excludePredicate))
{
    this->rootPaths.reserve(rootPaths.size());
    for (const QString &path : rootPaths) {
        this->rootPaths.append(QDir(path).absolutePath());
    }
    this->rootPaths.removeDuplicates();
}

VfsMonitorFileSystemWatcherPrivate::~VfsMonitorFileSystemWatcherPrivate()
{
    if (notifier) {
        notifier->setEnabled(false);
    }

    if (socketFd >= 0) {
        ::close(socketFd);
        socketFd = -1;
    }
}

bool VfsMonitorFileSystemWatcherPrivate::initMountPoints()
{
    mountPoints.clear();
    orderedMountPoints.clear();

    libmnt_table *mtab = mnt_new_table();
    if (!mtab)
        return false;

    if (mnt_table_parse_mtab(mtab, nullptr) < 0) {
        mnt_free_table(mtab);
        return false;
    }

    const QHash<int, MountEntry> byMountId = collectMountEntries(mtab);
    mnt_free_table(mtab);

    for (auto it = byMountId.cbegin(); it != byMountId.cend(); ++it) {
        const MountEntry &entry = it.value();
        if (!isParentChainUnderRoot(byMountId, entry))
            continue;

        mountPoints[entry.deviceId].append(entry.mountPoint);
        orderedMountPoints.append({ entry.deviceId, entry.mountPoint });
    }

    for (auto &points : mountPoints) {
        points.removeDuplicates();
        std::sort(points.begin(), points.end(),
                  [](const QString &left, const QString &right) {
                      return left.length() > right.length();
                  });
    }

    std::sort(orderedMountPoints.begin(), orderedMountPoints.end(),
              [](const MountPointAlias &left, const MountPointAlias &right) {
                  return left.mountPoint.length() > right.mountPoint.length();
              });

    return !mountPoints.isEmpty();
}

QString VfsMonitorFileSystemWatcherPrivate::resolveAndFilterFullPath(const char *absolutePath) const
{
    if (!absolutePath || absolutePath[0] == '\0')
        return {};

    const QString fullPath = QDir::cleanPath(QString::fromUtf8(absolutePath));
    if (!fullPath.startsWith('/'))
        return {};

    const QString directPath = filterDirectPath(rootPaths, excludePredicate, fullPath);
    if (!directPath.isNull())
        return directPath;

    for (const MountPointAlias &sourceAlias : orderedMountPoints) {
        if (!mountPointStartsWith(fullPath, sourceAlias.mountPoint))
            continue;

        const auto pointsIt = mountPoints.constFind(sourceAlias.deviceId);
        if (pointsIt == mountPoints.cend())
            return {};

        const QString suffix = (sourceAlias.mountPoint == "/")
                ? fullPath
                : fullPath.mid(sourceAlias.mountPoint.length());

        for (const QString &candidateMountPoint : pointsIt.value()) {
            const QString candidateFullPath = (candidateMountPoint == "/")
                    ? suffix
                    : candidateMountPoint + suffix;
            const QString filteredCandidate = filterDirectPath(rootPaths, excludePredicate, candidateFullPath);
            if (!filteredCandidate.isNull())
                return filteredCandidate;
        }

        // The longest matching source mount point wins. If its aliases do not
        // land inside a monitored root, do not fall back to shorter prefixes.
        return {};
    }

    return {};
}

QPair<QString, QString> VfsMonitorFileSystemWatcherPrivate::splitPath(const QString &fullPath)
{
    QFileInfo fi(fullPath);
    return qMakePair(fi.absolutePath(), fi.fileName());
}

bool VfsMonitorFileSystemWatcherPrivate::initDispatcher()
{
    Q_Q(VfsMonitorFileSystemWatcher);

    if (!initMountPoints()) {
        fmWarning() << "VfsMonitor: failed to initialize mount point aliases";
    }

    // The notifier runs on the GUI thread, so keep dispatcher reads nonblocking.
    socketFd = ::socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);
    if (socketFd < 0) {
        fmWarning() << "VfsMonitor: failed to create dispatcher socket:" << std::strerror(errno);
        return false;
    }

    sockaddr_un address {};
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, kDispatcherSocketPath, sizeof(address.sun_path) - 1);

    if (::connect(socketFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0) {
        fmWarning() << "VfsMonitor: deepin-anything event dispatcher not available:" << std::strerror(errno);
        ::close(socketFd);
        socketFd = -1;
        return false;
    }

    notifier = new QSocketNotifier(socketFd, QSocketNotifier::Read, q);
    QObject::connect(notifier, &QSocketNotifier::activated, q, [this]() {
        handleSocketMessage();
    });

    fmInfo() << "VfsMonitor: connected to deepin-anything event dispatcher";
    return true;
}

void VfsMonitorFileSystemWatcherPrivate::handleSocketMessage()
{
    if (socketFd < 0) {
        return;
    }

    auto *q = q_ptr;
    constexpr size_t kMinMessageSize = offsetof(DispatchEvent, eventPath) + 1;

    // Drain all queued packets for this wakeup so the notifier stays level-safe.
    while (true) {
        DispatchEvent event {};
        ssize_t received = -1;
        do {
            received = ::recv(socketFd, &event, sizeof(event), 0);
        } while (received < 0 && errno == EINTR);

        if (received == 0) {
            fmWarning() << "VfsMonitor: event dispatcher connection closed";
            if (notifier) {
                notifier->setEnabled(false);
            }
            ::close(socketFd);
            socketFd = -1;
            return;
        }

        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }

            fmWarning() << "VfsMonitor: failed to receive dispatcher event:" << std::strerror(errno);
            return;
        }

        if (static_cast<size_t>(received) < kMinMessageSize) {
            fmWarning() << "VfsMonitor: received short dispatcher message:" << received;
            continue;
        }

        event.eventPath[kDispatchMaxPathLen - 1] = '\0';

        const int act = event.action;
        const uint32_t cookie = event.cookie;
        const char *pathCStr = event.eventPath;

        if (act < ACT_NEW_FILE || act > ACT_UNMOUNT) {
            continue;
        }

        if (act == ACT_MOUNT || act == ACT_UNMOUNT) {
            if (!initMountPoints()) {
                fmWarning() << "VfsMonitor: failed to refresh mount point aliases";
            }
            continue;
        }

        if (act == ACT_RENAME_FROM_FILE || act == ACT_RENAME_FROM_FOLDER) {
            QString fullPath = resolveAndFilterFullPath(pathCStr);
            if (fullPath.isNull())
                continue;

            auto [parentPath, name] = splitPath(fullPath);
            RenameFromInfo info;
            info.path = parentPath;
            info.name = name;
            info.isDirectory = (act == ACT_RENAME_FROM_FOLDER);
            pendingRenames.insert(cookie, info);
            continue;
        }

        if (act == ACT_RENAME_TO_FILE || act == ACT_RENAME_TO_FOLDER) {
            const bool isDir = (act == ACT_RENAME_TO_FOLDER);
            auto it = pendingRenames.find(cookie);
            if (it != pendingRenames.end()) {
                QString fullPath = resolveAndFilterFullPath(pathCStr);
                if (!fullPath.isNull()) {
                    auto [parentPath, name] = splitPath(fullPath);
                    if (isDir)
                        Q_EMIT q->directoryMoved(it->path, it->name, parentPath, name);
                    else
                        Q_EMIT q->fileMoved(it->path, it->name, parentPath, name);
                } else {
                    if (isDir)
                        Q_EMIT q->directoryDeleted(it->path, it->name);
                    else
                        Q_EMIT q->fileDeleted(it->path, it->name);
                }
                pendingRenames.erase(it);
            } else {
                QString fullPath = resolveAndFilterFullPath(pathCStr);
                if (!fullPath.isNull()) {
                    auto [parentPath, name] = splitPath(fullPath);
                    if (isDir)
                        Q_EMIT q->directoryCreated(parentPath, name);
                    else
                        Q_EMIT q->fileCreated(parentPath, name);
                }
            }
            continue;
        }

        const QString fullPath = resolveAndFilterFullPath(pathCStr);
        if (fullPath.isNull()) {
            continue;
        }

        auto [parentPath, name] = splitPath(fullPath);

        switch (act) {
        case ACT_NEW_FILE:
        case ACT_NEW_LINK:
        case ACT_NEW_SYMLINK:
            Q_EMIT q->fileCreated(parentPath, name);
            break;
        case ACT_NEW_FOLDER:
            Q_EMIT q->directoryCreated(parentPath, name);
            break;
        case ACT_DEL_FILE:
            Q_EMIT q->fileDeleted(parentPath, name);
            break;
        case ACT_DEL_FOLDER:
            Q_EMIT q->directoryDeleted(parentPath, name);
            break;
        case ACT_RENAME_FILE:
            Q_EMIT q->fileCreated(parentPath, name);
            break;
        case ACT_RENAME_FOLDER:
            Q_EMIT q->directoryCreated(parentPath, name);
            break;
        default:
            break;
        }

        // Clean up orphaned RENAME_FROM entries.
        static constexpr int kPendingRenameCleanupThreshold = 1000;
        if (pendingRenames.size() > kPendingRenameCleanupThreshold) {
            fmWarning() << "VfsMonitor: pending rename table too large ("
                        << pendingRenames.size() << "), clearing";
            pendingRenames.clear();
        }
    }
}

// ========== VfsMonitorFileSystemWatcher ==========

VfsMonitorFileSystemWatcher::VfsMonitorFileSystemWatcher(const QStringList &rootPaths,
                                                         PathExcludePredicate excludePredicate,
                                                         QObject *parent)
    : QObject(parent), d_ptr(new VfsMonitorFileSystemWatcherPrivate(rootPaths, std::move(excludePredicate), this))
{
}

VfsMonitorFileSystemWatcher::~VfsMonitorFileSystemWatcher()
{
}

VfsMonitorFileSystemWatcher *VfsMonitorFileSystemWatcher::create(const QStringList &rootPaths,
                                                                 PathExcludePredicate excludePredicate,
                                                                 QObject *parent)
{
    auto *watcher = new VfsMonitorFileSystemWatcher(rootPaths, std::move(excludePredicate), parent);

    if (!watcher->d_func()->initDispatcher()) {
        delete watcher;
        return nullptr;
    }

    return watcher;
}

SERVICETEXTINDEX_END_NAMESPACE
