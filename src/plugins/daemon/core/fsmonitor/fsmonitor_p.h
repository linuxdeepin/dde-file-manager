// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSMONITOR_P_H
#define FSMONITOR_P_H

#include "fsmonitor.h"

#include <QFileInfo>
#include <QQueue>
#include <QTimer>
#include <QFuture>
#include <QThreadPool>
#include <QtConcurrent>

namespace Dtk {
namespace Core {
class DFileSystemWatcher;
}
}

DAEMONPCORE_BEGIN_NAMESPACE

class FSMonitorPrivate
{
public:
    FSMonitorPrivate(FSMonitor *qq);
    ~FSMonitorPrivate();

    // Initializes the monitor with the root path
    bool init(const QString &rootPath);

    // Start monitoring using DFileSystemWatcher
    bool startMonitoring();

    // Stop all monitoring
    void stopMonitoring();

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

    // Process a directory in the queue (non-blocking)
    void processNextPendingDirectory();

    // Handle file creation event
    void handleFileCreated(const QString &path, const QString &name);

    // Handle file deletion event
    void handleFileDeleted(const QString &path, const QString &name);

    // Handle file modification event
    void handleFileModified(const QString &path, const QString &name);

    // Handle file moved event
    void handleFileMoved(const QString &fromPath, const QString &fromName,
                         const QString &toPath, const QString &toName);

    // Detect if a path is a directory
    bool isDirectory(const QString &path, const QString &name) const;

    // Debug and error logging helpers
    void logDebug(const QString &message) const;
    void logError(const QString &message) const;

    // Data members
    FSMonitor *q_ptr;
    QScopedPointer<Dtk::Core::DFileSystemWatcher> watcher;

    QString rootPath;
    QSet<QString> watchedDirectories;
    QSet<QString> blacklistedPaths;
    QQueue<QString> pendingDirectories;
    QThreadPool threadPool;
    QTimer processingTimer;
    bool active { false };

    // Resource limits
    double maxUsagePercentage { 0.5 };   // Default to 50% of available watches
    int maxWatches { -1 };   // Will be determined at runtime

    // Default blacklisted directories
    static const QStringList defaultBlacklistedDirs;
};

DAEMONPCORE_END_NAMESPACE

#endif   // FSMONITOR_P_H
