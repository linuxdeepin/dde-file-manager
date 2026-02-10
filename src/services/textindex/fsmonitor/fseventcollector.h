// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSEVENTCOLLECTOR_H
#define FSEVENTCOLLECTOR_H

#include "service_textindex_global.h"
#include "fsmonitor.h"

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QSet>
#include <QTimer>
#include <QMutex>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FSEventCollectorPrivate;

// FSEventCollector: A file system event collector for batched processing
// Collects and aggregates events from FSMonitor over configurable time intervals
// Used to provide a lower-frequency source of filesystem events for indexing
//
// Features:
// - Batches file system events over configurable time periods
// - Smart event merging (e.g., create+delete cancels out both)
// - Categorizes events into created, deleted, and modified files
// - Limits maximum collection size to prevent resource exhaustion
// - Provides customizable collection intervals
class FSEventCollector : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FSEventCollector)

public:
    // Constructor with explicit FSMonitor reference
    explicit FSEventCollector(FSMonitor &monitor, QObject *parent = nullptr);

    // Constructor with default FSMonitor (uses FSMonitor::instance())
    explicit FSEventCollector(QObject *parent = nullptr);

    ~FSEventCollector() override;

    // Initialize the collector with monitor root paths
    bool initialize(const QStringList &rootPaths);

    // Start collecting events
    bool start();

    // Stop collecting events
    void stop();

    // Check if collector is active
    bool isActive() const;

    // Set collection interval in seconds (default: 180 seconds)
    void setCollectionInterval(int seconds);

    // Get current collection interval in seconds
    int collectionInterval() const;

    // Set maximum number of events to collect before auto-flush (default: 10000)
    void setMaxEventCount(int count);

    // Get current maximum event count
    int maxEventCount() const;

    // Manually flush collected events (emits signals and clears cache)
    void flushEvents();

    // Clear all cached events without emitting signals
    void clearEvents();

    // Get current lists of collected events
    QStringList createdFiles() const;
    QStringList deletedFiles() const;
    QStringList modifiedFiles() const;
    
    // Get current moved files mapping (fromPath -> toPath)
    QHash<QString, QString> movedFiles() const;

    // Get current count of collected events
    int createdFilesCount() const;
    int deletedFilesCount() const;
    int modifiedFilesCount() const;
    int totalEventsCount() const;
    
    // Get count of moved files
    int movedFilesCount() const;

Q_SIGNALS:
    // Emitted when collection interval expires with batched events
    void filesCreated(const QStringList &paths);
    void filesDeleted(const QStringList &paths);
    void filesModified(const QStringList &paths);
    
    // Emitted when files/directories are moved/renamed (fromPath -> toPath mapping)
    void filesMoved(const QHash<QString, QString> &movedPaths);
    
    void flushFinished();

    // Emitted when maximum event count is reached
    void maxEventCountReached(int count);

    // Emitted when an error occurs
    void errorOccurred(const QString &errorMessage);

private:
    // Private implementation
    QScopedPointer<FSEventCollectorPrivate> d_ptr;
    Q_DECLARE_PRIVATE(FSEventCollector)
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSEVENTCOLLECTOR_H
