// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSMONITORWORKER_H
#define FSMONITORWORKER_H

#include "service_textindex_global.h"

#include <QObject>
#include <QDir>
#include <QStringList>
#include <QFuture>
#include <QFutureWatcher>
#include <QSet>

#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FSMonitorWorker : public QObject
{
    Q_OBJECT

public:
    explicit FSMonitorWorker(QObject *parent = nullptr);
    ~FSMonitorWorker() override;

    // Set exclusion checker function (moved to regular method to avoid MOC issues)
    void setExclusionChecker(const std::function<bool(const QString &)> &checker);

    void setMaxFastScanResults(int max);

    // Whether fast directory scan is currently in progress
    bool isFastScanInProgress() const { return fastScanInProgress; }

public slots:
    // Process a directory and find its subdirectories
    void processDirectory(const QString &path);

    // Try to get all directories using fast scan method (new feature)
    void tryFastDirectoryScan();

signals:
    // Emitted when a directory should be watched
    void directoryToWatch(const QString &path);

    // Emitted when subdirectories are found
    void subdirectoriesFound(const QStringList &directories);

    // Emitted when a batch of directories should be watched
    void directoriesBatchToWatch(const QStringList &paths);

    // Emitted when fast scan completes (whether success or failure)
    void fastScanCompleted(bool success);

private slots:
    void handleFastScanResult();

private:
    // Function to check if a path should be excluded
    std::function<bool(const QString &)> exclusionChecker;

    // Whether fast scan is currently in progress
    bool fastScanInProgress { false };

    // Future watcher for async operations
    QFutureWatcher<QStringList> *futureWatcher { nullptr };

    int maxResultsCount { 65536 };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FSMONITORWORKER_H
