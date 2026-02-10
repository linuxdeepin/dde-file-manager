// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSMONITOR_H
#define FSMONITOR_H

#include "service_textindex_global.h"

#include <QObject>
#include <QStringList>
#include <QSet>
#include <QHash>
#include <QMutex>
#include <QDir>

namespace Dtk {
namespace Core {
class DFileSystemWatcher;
}
}

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FSMonitorPrivate;

// FSMonitor: A recursive file system monitor based on DFileSystemWatcher
// Monitors user's home directory and handles file/directory events for indexing
//
// Features:
// - Recursively watches directories
// - Manages system resource usage by limiting watch count
// - Excludes blacklisted, system, external, and network mounts automatically
// - Skips symbolic links to prevent circular watch issues
// - Respects the system hidden files setting - ignores hidden files when configured
// - Can optionally use fast directory scanning at startup for performance
class FSMonitor : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FSMonitor)

public:
    // Singleton access
    static FSMonitor &instance();

    // Initialize monitoring with root paths
    bool initialize(const QStringList &rootPaths);

    // Start monitoring
    bool start();

    // Stop monitoring
    void stop();

    // Check if monitoring is active
    bool isActive() const;

    // Add a path to blacklist (won't be monitored)
    void addBlacklistedPath(const QString &path);

    // Add multiple paths to blacklist
    void addBlacklistedPaths(const QStringList &paths);

    // Remove a path from blacklist
    void removeBlacklistedPath(const QString &path);

    // Get current blacklisted paths
    QStringList blacklistedPaths() const;

    // Set maximum resource usage percentage (0.0-1.0, default 0.5)
    void setMaxResourceUsage(double percentage);

    // Get current resource usage percentage
    double maxResourceUsage() const;

    // Get current watch count
    int currentWatchCount() const;

    // Get maximum available watch count from system
    int maxAvailableWatchCount() const;

    // Enable or disable fast directory scanning (must be called before start)
    void setUseFastScan(bool enable);

    // Check if fast scanning is enabled
    bool useFastScan() const;

Q_SIGNALS:
    // Emitted when a file is created
    void fileCreated(const QString &path, const QString &name);

    // Emitted when a file is deleted
    void fileDeleted(const QString &path, const QString &name);

    // Emitted when a file is modified
    void fileModified(const QString &path, const QString &name);

    // Emitted when a file is moved
    void fileMoved(const QString &fromPath, const QString &fromName,
                   const QString &toPath, const QString &toName);

    // Emitted when a directory is created
    void directoryCreated(const QString &path, const QString &name);

    // Emitted when a directory is deleted
    void directoryDeleted(const QString &path, const QString &name);

    // Emitted when a directory is moved
    void directoryMoved(const QString &fromPath, const QString &fromName,
                        const QString &toPath, const QString &toName);

    // Emitted when resource usage reaches the limit
    void resourceLimitReached(int currentCount, int maxCount);

    // Emitted when an error occurs
    void errorOccurred(const QString &errorMessage);

private:
    explicit FSMonitor(QObject *parent = nullptr);
    ~FSMonitor() override;

    // Private implementation
    QScopedPointer<FSMonitorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(FSMonitor)
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSMONITOR_H
