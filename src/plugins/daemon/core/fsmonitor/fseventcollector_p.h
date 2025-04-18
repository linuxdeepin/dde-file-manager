// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSEVENTCOLLECTOR_P_H
#define FSEVENTCOLLECTOR_P_H

#include "fseventcollector.h"

#include <QTimer>
#include <QSet>
#include <QDateTime>

DAEMONPCORE_BEGIN_NAMESPACE

class FSEventCollectorPrivate
{
public:
    FSEventCollectorPrivate(FSEventCollector *qq, FSMonitor &monitor);
    ~FSEventCollectorPrivate();

    // Initialize the collector with monitor root path
    bool init(const QString &rootPath);

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

    // Debug and error logging helpers
    void logDebug(const QString &message) const;
    void logError(const QString &message) const;

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

    // Root path being monitored
    QString rootPath;

    // Maximum event count before auto-flush (default: 10000)
    int maxEvents { 10000 };

    // Active state
    bool active { false };

    QSet<QString> createdFilesList;
    QSet<QString> deletedFilesList;
    QSet<QString> modifiedFilesList;
};

DAEMONPCORE_END_NAMESPACE

#endif   // FSEVENTCOLLECTOR_P_H
