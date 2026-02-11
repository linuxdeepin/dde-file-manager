// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSMONITOR_P_H
#define FSMONITOR_P_H

#include "fsmonitor.h"
#include "fsmonitorworker.h"
#include "utils/pathexcludematcher.h"

#include <QFileInfo>
#include <QSet>
#include <QThread>
#include <QTimer>

namespace Dtk {
namespace Core {
class DFileSystemWatcher;
}
}

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FSMonitorPrivate
{
public:
    FSMonitorPrivate(FSMonitor *qq);
    ~FSMonitorPrivate();

    // Initialize the monitor with root paths
    bool init(const QStringList &rootPaths);
    bool init(const QString &rootPath);   // For backward compatibility

    // Start monitoring using DFileSystemWatcher
    bool startMonitoring();

    // Stop all monitoring
    void stopMonitoring();

    // Process the root directories using traditional method
    void travelRootDirectories();

    // Get the maximum number of watches supported by the system
    int getMaxUserWatches() const;

    // Check if a path should be excluded (blacklisted or external mount)
    bool shouldExcludePath(const QString &path) const;

    // Check if a path is on an external mount
    bool isExternalMount(const QString &path) const;

    // Check if a path is a symbolic link
    bool isSymbolicLink(const QString &path) const;

    // Check if hidden config opened
    bool showHidden() const;

    // Add a directory and all its subdirectories to the watch
    void addDirectoryRecursively(const QString &path);

    // Add a single directory to watch
    bool addWatchForDirectory(const QString &path);

    // Remove a directory from watch
    void removeWatchForDirectory(const QString &path);

    // Check if we're below the system watch limit
    bool isWithinWatchLimit() const;

    // Parse and connect watcher signals
    void setupWatcherConnections();

    // Set up the worker thread and connections
    void setupWorkerThread();

    // Handle file creation event
    void handleFileCreated(const QString &path, const QString &name);

    // Handle file deletion event
    void handleFileDeleted(const QString &path, const QString &name);

    // Handle file modification event
    void handleFileModified(const QString &path, const QString &name);

    // Handle file moved event
    void handleFileMoved(const QString &fromPath, const QString &fromName,
                         const QString &toPath, const QString &toName);

    // Handle fast scan completion
    void handleFastScanCompleted(bool success);

    // Handle batch of directories to watch
    void handleDirectoriesBatch(const QStringList &paths);

    // Detect if a path is a directory
    bool isDirectory(const QString &path, const QString &name) const;

    // Fast scan control
    bool useFastScan { true };   // Whether to try fast scan first

    // Data members
    FSMonitor *q_ptr;
    QScopedPointer<Dtk::Core::DFileSystemWatcher> watcher;

    // Worker thread members
    QThread workerThread;
    FSMonitorWorker *worker { nullptr };

    QStringList rootPaths;
    QSet<QString> watchedDirectories;
    PathExcludeMatcher excludeMatcher;
    bool active { false };

    // Resource limits
    double maxUsagePercentage { 0.5 };   // Default to 50% of available watches
    int maxWatches { -1 };   // Will be determined at runtime
    
    // Resource limit tracking
    bool resourceLimitReached { false };   // Flag to track if watch limit has been reached
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSMONITOR_P_H
