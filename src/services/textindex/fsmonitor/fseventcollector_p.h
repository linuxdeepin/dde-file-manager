// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSEVENTCOLLECTOR_P_H
#define FSEVENTCOLLECTOR_P_H

#include "fseventcollector.h"

#include <QTimer>
#include <QSet>
#include <QDateTime>
#include <QHash>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FSEventCollectorPrivate
{
public:
    FSEventCollectorPrivate(FSEventCollector *qq, FSMonitor &monitor);
    ~FSEventCollectorPrivate();

    // Initialize the collector with monitor root path
    bool init(const QStringList &rootPaths);

    // Start collecting events
    bool startCollecting();

    // Stop collecting events
    void stopCollecting();

    // Process file created event
    void handleFileCreated(const QString &path, const QString &name);

    // Process file deleted event
    void handleFileDeleted(const QString &path, const QString &name);

    // Process file modified event
    void handleFileModified(const QString &path, const QString &name);

    // Process file moved event
    void handleFileMoved(const QString &fromPath, const QString &fromName,
                         const QString &toPath, const QString &toName);

    // Process directory created event
    void handleDirectoryCreated(const QString &path, const QString &name);

    // Process directory deleted event
    void handleDirectoryDeleted(const QString &path, const QString &name);

    // Process directory moved event
    void handleDirectoryMoved(const QString &fromPath, const QString &fromName,
                              const QString &toPath, const QString &toName);

    // Flush collected events (emit signals and clear)
    void flushCollectedEvents();

    // Check if max event count exceeded
    bool isMaxEventCountExceeded() const;

    // Remove redundant file entries that are under directories already in the list
    void removeRedundantEntries(QSet<QString> &filesList);

    // Check if a path is a child of any path in the given set
    bool isChildOfAnyPath(const QString &path, const QSet<QString> &pathSet) const;

    // Check if path is a directory
    bool isDirectory(const QString &path) const;

    // Remove redundant entries from all event lists
    void cleanupRedundantEntries();

    // Remove entries covered by deleted directories
    void removeEntriesCoveredByDirectories();

    // Check if a file should be indexed based on its extension
    bool shouldIndexFile(const QString &path) const;

    // Normalize path for consistent handling
    QString normalizePath(const QString &dirPath, const QString &fileName) const;

    // Build full path from components
    QString buildPath(const QString &dirPath, const QString &fileName) const;

    // Data members
    FSEventCollector *q_ptr;

    // The FSMonitor reference (no longer a singleton)
    FSMonitor &fsMonitor;

    // Collection timer
    QTimer collectionTimer;

    // Collection interval in milliseconds (default: 180000ms = 3 minutes)
    int collectionIntervalMs { 180000 };

    // Root paths being monitored
    QStringList rootPaths;

    // Maximum event count before auto-flush (default: 10000)
    int maxEvents { 10000 };

    // Active state
    bool active { false };

    QSet<QString> createdFilesList;
    QSet<QString> deletedFilesList;
    QSet<QString> modifiedFilesList;

    // New: Track moved/renamed files separately for efficient index updates
    QHash<QString, QString> movedFilesList;   // fromPath -> toPath mapping

    // Marker for deleted directories
    QSet<QString> deletedDirectoriesMarker;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSEVENTCOLLECTOR_P_H
