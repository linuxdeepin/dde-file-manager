// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vfsmonitorwatcher_p.h"

#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/attr.h>
#include <netlink/msg.h>

#include <libmount.h>
#include <sys/sysmacros.h>

#include <cstring>
#include <algorithm>

SERVICETEXTINDEX_BEGIN_NAMESPACE

// Netlink family and multicast group names (matching vfs_genl.h)
static constexpr char kVfsMonitorFamilyName[] = "vfsmonitor";
static constexpr char kVfsMonitorMcgDentry[] = "vfsmonitor_de";

// --- Static C callback for libnl ---

namespace {

struct CallbackContext
{
    VfsMonitorFileSystemWatcherPrivate *d { nullptr };
};

// Netlink attribute policy for parsing vfs_monitor messages.
struct nla_policy vfsPolicy[VFSMONITOR_A_MAX + 1];

void initVfsPolicy()
{
    std::memset(vfsPolicy, 0, sizeof(vfsPolicy));
    vfsPolicy[VFSMONITOR_A_ACT].type = NLA_U8;
    vfsPolicy[VFSMONITOR_A_COOKIE].type = NLA_U32;
    vfsPolicy[VFSMONITOR_A_MAJOR].type = NLA_U16;
    vfsPolicy[VFSMONITOR_A_MINOR].type = NLA_U8;
    vfsPolicy[VFSMONITOR_A_PATH].type = NLA_NUL_STRING;
    vfsPolicy[VFSMONITOR_A_PATH].maxlen = 4096;
}

int vfsMonitorMsgCallback(struct nl_msg *msg, void *arg)
{
    auto *ctx = static_cast<CallbackContext *>(arg);
    auto *d = ctx->d;

    if (!d) {
        return NL_SKIP;
    }

    struct nlattr *tb[VFSMONITOR_A_MAX + 1];
    std::memset(tb, 0, sizeof(tb));

    struct nlmsghdr *nh = nlmsg_hdr(msg);
    int err = genlmsg_parse(nh, 0, tb, VFSMONITOR_A_MAX, vfsPolicy);
    if (err < 0) {
        return NL_SKIP;
    }

    if (!tb[VFSMONITOR_A_PATH]) {
        return NL_SKIP;
    }

    // Extract attributes.
    uint8_t act = tb[VFSMONITOR_A_ACT] ? nla_get_u8(tb[VFSMONITOR_A_ACT]) : 0;
    uint32_t cookie = tb[VFSMONITOR_A_COOKIE] ? nla_get_u32(tb[VFSMONITOR_A_COOKIE]) : 0;
    uint16_t major = tb[VFSMONITOR_A_MAJOR] ? nla_get_u16(tb[VFSMONITOR_A_MAJOR]) : 0;
    uint8_t minor = tb[VFSMONITOR_A_MINOR] ? nla_get_u8(tb[VFSMONITOR_A_MINOR]) : 0;
    const char *pathCStr = reinterpret_cast<const char *>(nla_data(tb[VFSMONITOR_A_PATH]));

    // Skip non-filesystem events.
    if (act > ACT_UNMOUNT) {
        return NL_OK;
    }

    // Skip mount/unmount events.
    if (act == ACT_MOUNT || act == ACT_UNMOUNT) {
        return NL_OK;
    }

    // Filter events from devices that have child mounts (overlay lower layers).
    dev_t deviceId = makedev(major, minor);
    if (d->hasChildMountPoints(deviceId)) {
        return NL_OK;
    }

    // RENAME_FROM events must be stored before filtering, because the paired
    // RENAME_TO may land in an excluded path (e.g., trash). We need the
    // RENAME_FROM info to emit a delete when that happens.
    if (act == ACT_RENAME_FROM_FILE || act == ACT_RENAME_FROM_FOLDER) {
        QString fullPath = d->resolveAndFilterFullPath(deviceId, pathCStr);
        if (fullPath.isNull())
            return NL_OK;  // Source itself is excluded, no point tracking.

        auto [parentPath, name] = VfsMonitorFileSystemWatcherPrivate::splitPath(fullPath);

        RenameFromInfo info;
        info.path = parentPath;
        info.name = name;
        info.isDirectory = (act == ACT_RENAME_FROM_FOLDER);
        d->pendingRenames.insert(cookie, info);
        return NL_OK;
    }

    // RENAME_TO events need independent path resolution because the target may
    // be filtered (e.g., moved to trash). In that case we emit a delete for the
    // source instead of a move — the generic fullPath.isNull() gate below would
    // swallow this event before we could check pendingRenames.
    if (act == ACT_RENAME_TO_FILE || act == ACT_RENAME_TO_FOLDER) {
        auto *q = d->q_ptr;
        const bool isDir = (act == ACT_RENAME_TO_FOLDER);

        auto it = d->pendingRenames.find(cookie);
        if (it != d->pendingRenames.end()) {
            QString fullPath = d->resolveAndFilterFullPath(deviceId, pathCStr);
            if (!fullPath.isNull()) {
                auto [parentPath, name] = VfsMonitorFileSystemWatcherPrivate::splitPath(fullPath);
                if (isDir)
                    Q_EMIT q->directoryMoved(it->path, it->name, parentPath, name);
                else
                    Q_EMIT q->fileMoved(it->path, it->name, parentPath, name);
            } else {
                // Target is filtered (e.g., moved to trash) → source is gone.
                if (isDir)
                    Q_EMIT q->directoryDeleted(it->path, it->name);
                else
                    Q_EMIT q->fileDeleted(it->path, it->name);
            }
            d->pendingRenames.erase(it);
        } else {
            // Unpaired RENAME_TO: treat as creation if in monitored area.
            QString fullPath = d->resolveAndFilterFullPath(deviceId, pathCStr);
            if (!fullPath.isNull()) {
                auto [parentPath, name] = VfsMonitorFileSystemWatcherPrivate::splitPath(fullPath);
                if (isDir)
                    Q_EMIT q->directoryCreated(parentPath, name);
                else
                    Q_EMIT q->fileCreated(parentPath, name);
            }
        }
        return NL_OK;
    }

    // For all other events, reconstruct and filter the full path.
    QString fullPath = d->resolveAndFilterFullPath(deviceId, pathCStr);
    if (fullPath.isNull()) {
        return NL_OK;
    }

    auto *q = d->q_ptr;
    auto [parentPath, name] = VfsMonitorFileSystemWatcherPrivate::splitPath(fullPath);

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

    // Synthetic events (kernel-paired rename).
    case ACT_RENAME_FILE:
        Q_EMIT q->fileCreated(parentPath, name);
        break;

    case ACT_RENAME_FOLDER:
        Q_EMIT q->directoryCreated(parentPath, name);
        break;

    default:
        break;
    }

    return NL_OK;
}

static CallbackContext s_callbackCtx { nullptr };

static bool registerMessageCallback(nl_sock *sock, VfsMonitorFileSystemWatcherPrivate *d)
{
    s_callbackCtx.d = d;

    int ret = nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM,
                                  vfsMonitorMsgCallback, &s_callbackCtx);
    if (ret != 0) {
        return false;
    }

    return true;
}

static void clearMessageCallback()
{
    s_callbackCtx.d = nullptr;
}

}   // anonymous namespace

// ========== VfsMonitorFileSystemWatcherPrivate ==========

VfsMonitorFileSystemWatcherPrivate::VfsMonitorFileSystemWatcherPrivate(
        const QStringList &rootPaths,
        VfsMonitorFileSystemWatcher::PathExcludePredicate excludePredicate,
        VfsMonitorFileSystemWatcher *qq)
    : q_ptr(qq), rootPaths(rootPaths), excludePredicate(std::move(excludePredicate))
{
}

VfsMonitorFileSystemWatcherPrivate::~VfsMonitorFileSystemWatcherPrivate()
{
    if (notifier) {
        notifier->setEnabled(false);
    }

    clearMessageCallback();

    if (nlSock) {
        nl_socket_free(nlSock);
        nlSock = nullptr;
    }
}

bool VfsMonitorFileSystemWatcherPrivate::initMountPoints()
{
    mountPoints.clear();
    devicesWithChildren.clear();

    struct libmnt_table *mtab = mnt_new_table();
    if (!mtab) {
        return false;
    }

    if (mnt_table_parse_mtab(mtab, nullptr) < 0) {
        mnt_free_table(mtab);
        return false;
    }

    // Collect ALL mount entries with their parent mount IDs.
    // We keep both real mounts (root="/") and bind mounts (root="/some/path").
    // For bind mounts, the kernel event's dentry_path_raw is relative to the
    // original mount point (root="/"), so we still use the real mount point
    // for path reconstruction. But we also store bind mount targets so that
    // resolveFullPath() can try them for rootPath matching.
    //
    // Structure: mountId -> {deviceId, parentMountId, mountPoint, isBindMount}
    struct MountEntry {
        dev_t deviceId;
        int parentMountId;
        QString mountPoint;
        bool isBindMount;
    };
    QHash<int, MountEntry> byMountId;

    struct libmnt_iter *iter = mnt_new_iter(MNT_ITER_FORWARD);
    struct libmnt_fs *fs;
    while (mnt_table_next_fs(mtab, iter, &fs) == 0) {
        const char *target = mnt_fs_get_target(fs);
        if (!target)
            continue;

        int mountId = mnt_fs_get_id(fs);
        dev_t devId = mnt_fs_get_devno(fs);
        int parentId = mnt_fs_get_parent_id(fs);

        // Determine if this is a bind mount (root != "/").
        bool isBind = (qstrcmp(mnt_fs_get_root(fs), "/") != 0);

        MountEntry entry;
        entry.deviceId = devId;
        entry.parentMountId = parentId;
        entry.mountPoint = QString::fromUtf8(target);
        entry.isBindMount = isBind;

        byMountId.insert(mountId, entry);
    }

    mnt_free_iter(iter);
    mnt_free_table(mtab);

    // Group mount points by device ID.
    // Only include entries whose parent chain leads to "/" (same as deepin-anything).
    for (auto it = byMountId.cbegin(); it != byMountId.cend(); ++it) {
        const auto &entry = it.value();

        if (!entry.isBindMount) {
            // Real mount: root == "/". Check parent chain if not root fs.
            if (entry.mountPoint != "/") {
                int pid = entry.parentMountId;
                bool allRoot = true;
                while (pid > 0) {
                    auto parentIt = byMountId.find(pid);
                    if (parentIt == byMountId.end()) {
                        allRoot = false;
                        break;
                    }
                    if (parentIt->mountPoint == "/") {
                        break;
                    }
                    pid = parentIt->parentMountId;
                }
                if (!allRoot)
                    continue;
            }
        } else {
            // Bind mount: verify its parent (the original mount) has a valid chain.
            int pid = entry.parentMountId;
            bool allRoot = true;
            while (pid > 0) {
                auto parentIt = byMountId.find(pid);
                if (parentIt == byMountId.end()) {
                    allRoot = false;
                    break;
                }
                if (parentIt->mountPoint == "/") {
                    break;
                }
                pid = parentIt->parentMountId;
            }
            if (!allRoot)
                continue;
        }

        mountPoints[entry.deviceId].append(entry.mountPoint);
    }

    // Sort each device's mount points by length (longest first).
    // This ensures more specific paths (e.g., "/home") are tried before
    // less specific ones (e.g., "/persistent/home").
    for (auto &points : mountPoints) {
        std::sort(points.begin(), points.end(),
                  [](const QString &a, const QString &b) {
                      return a.length() > b.length();
                  });
    }

    // Build child mount device set for lowerfs/overlay filtering.
    for (const auto &entry : std::as_const(byMountId)) {
        if (entry.parentMountId > 0 && !entry.isBindMount) {
            auto parentIt = byMountId.find(entry.parentMountId);
            if (parentIt != byMountId.end()) {
                devicesWithChildren.insert(parentIt->deviceId);
            }
        }
    }

    int totalPoints = 0;
    for (const auto &pts : std::as_const(mountPoints)) {
        totalPoints += pts.size();
    }
    fmInfo() << "VfsMonitor: loaded" << mountPoints.size()
             << "devices," << totalPoints << "mount points,"
             << devicesWithChildren.size() << "devices with children";
    return !mountPoints.isEmpty();
}

bool VfsMonitorFileSystemWatcherPrivate::hasChildMountPoints(dev_t deviceId) const
{
    return devicesWithChildren.contains(deviceId);
}

QString VfsMonitorFileSystemWatcherPrivate::resolveAndFilterFullPath(dev_t deviceId,
                                                                  const char *relativePath) const
{
    auto it = mountPoints.find(deviceId);
    if (it == mountPoints.end())
        return {};

    const QStringList &points = it.value();
    const QString relPath = QString::fromUtf8(relativePath);

    // Try each mount point (sorted longest first).
    // Return the first one that falls under a monitored root path
    // and passes the exclude predicate.
    for (const QString &mp : points) {
        QString fullPath;
        if (mp == "/") {
            fullPath = relPath;
        } else {
            fullPath = mp + relPath;
        }

        // Check if under a monitored root path.
        bool inRoot = false;
        for (const QString &root : rootPaths) {
            if (fullPath.startsWith(root)) {
                if (root.endsWith('/') || fullPath.length() == root.length()
                    || fullPath.at(root.length()) == '/') {
                    inRoot = true;
                    break;
                }
            }
        }
        if (!inRoot)
            continue;

        // Check exclude predicate (symlinks, hidden files, blacklists).
        if (excludePredicate && excludePredicate(fullPath))
            continue;

        return fullPath;
    }

    return {};
}

QPair<QString, QString> VfsMonitorFileSystemWatcherPrivate::splitPath(const QString &fullPath)
{
    QFileInfo fi(fullPath);
    return qMakePair(fi.absolutePath(), fi.fileName());
}

bool VfsMonitorFileSystemWatcherPrivate::initNetlink()
{
    Q_Q(VfsMonitorFileSystemWatcher);

    // Build mount point cache first (no netlink dependency).
    if (!initMountPoints()) {
        fmWarning() << "VfsMonitor: failed to initialize mount points";
    }

    nlSock = nl_socket_alloc();
    if (!nlSock) {
        fmWarning() << "VfsMonitor: failed to allocate netlink socket";
        return false;
    }

    if (genl_connect(nlSock) != 0) {
        fmWarning() << "VfsMonitor: genl_connect failed";
        nl_socket_free(nlSock);
        nlSock = nullptr;
        return false;
    }

    // Set receive buffer to system maximum to prevent event loss.
    QFile rmemFile("/proc/sys/net/core/rmem_max");
    if (rmemFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&rmemFile);
        bool ok = false;
        int maxRcvbuf = in.readLine().trimmed().toInt(&ok);
        if (ok && maxRcvbuf > 0) {
            if (nl_socket_set_buffer_size(nlSock, maxRcvbuf, 0) != 0) {
                fmWarning() << "VfsMonitor: failed to set socket buffer size";
            }
        }
        rmemFile.close();
    }

    nl_socket_disable_seq_check(nlSock);
    nl_socket_disable_auto_ack(nlSock);

    int mcgrpId = genl_ctrl_resolve_grp(nlSock, kVfsMonitorFamilyName, kVfsMonitorMcgDentry);
    if (mcgrpId < 0) {
        fmWarning() << "VfsMonitor: vfs_monitor kernel module not available";
        nl_socket_free(nlSock);
        nlSock = nullptr;
        return false;
    }

    if (nl_socket_add_membership(nlSock, mcgrpId) != 0) {
        fmWarning() << "VfsMonitor: failed to join multicast group";
        nl_socket_free(nlSock);
        nlSock = nullptr;
        return false;
    }

    initVfsPolicy();

    if (!registerMessageCallback(nlSock, this)) {
        nl_socket_free(nlSock);
        nlSock = nullptr;
        return false;
    }

    int fd = nl_socket_get_fd(nlSock);
    notifier = new QSocketNotifier(fd, QSocketNotifier::Read, q);
    QObject::connect(notifier, &QSocketNotifier::activated, q, [this]() {
        handleNetlinkMessage();
    });

    fmInfo() << "VfsMonitor: connected to vfs_monitor kernel module";
    return true;
}

void VfsMonitorFileSystemWatcherPrivate::handleNetlinkMessage()
{
    int ret = nl_recvmsgs_default(nlSock);
    if (ret < 0 && ret != -NLE_AGAIN) {
        fmWarning() << "VfsMonitor: nl_recvmsgs_default failed, error:" << ret;
    }

    // Clean up orphaned RENAME_FROM entries.
    static constexpr int kPendingRenameCleanupThreshold = 1000;
    if (pendingRenames.size() > kPendingRenameCleanupThreshold) {
        fmWarning() << "VfsMonitor: pending rename table too large ("
                    << pendingRenames.size() << "), clearing";
        pendingRenames.clear();
    }
}

// ========== VfsMonitorFileSystemWatcher ==========

VfsMonitorFileSystemWatcher::VfsMonitorFileSystemWatcher(const QStringList &rootPaths,
                                                         PathExcludePredicate excludePredicate,
                                                         QObject *parent)
    : QObject(parent)
    , d_ptr(new VfsMonitorFileSystemWatcherPrivate(rootPaths, std::move(excludePredicate), this))
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

    if (!watcher->d_func()->initNetlink()) {
        delete watcher;
        return nullptr;
    }

    return watcher;
}

SERVICETEXTINDEX_END_NAMESPACE
