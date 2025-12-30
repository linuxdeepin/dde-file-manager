// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsmonitorworker.h"

#include <dfm-base/base/application/application.h>

#include <dfm-search/searchengine.h>
#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QMetaObject>
#include <QtConcurrent>
#include <QFuture>

DFMBASE_USE_NAMESPACE
DFM_SEARCH_USE_NS

SERVICETEXTINDEX_BEGIN_NAMESPACE

FSMonitorWorker::FSMonitorWorker(QObject *parent)
    : QObject(parent)
{
    // Default exclusion checker (excludes nothing)
    exclusionChecker = [](const QString &) { return false; };

    // Initialize future watcher
    futureWatcher = new QFutureWatcher<QStringList>(this);
    connect(futureWatcher, &QFutureWatcher<QStringList>::finished,
            this, &FSMonitorWorker::handleFastScanResult);
}

FSMonitorWorker::~FSMonitorWorker()
{
    if (futureWatcher && futureWatcher->isRunning()) {
        futureWatcher->cancel();
        futureWatcher->waitForFinished();
    }
}

void FSMonitorWorker::processDirectory(const QString &path)
{
    if (path.isEmpty() || !QFileInfo(path).isDir()) {
        return;
    }

    // Skip excluded directories
    if (exclusionChecker(path)) {
        return;
    }

    // Emit signal to add this directory to watch
    emit directoryToWatch(path);

    // Find subdirectories
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (!entries.isEmpty()) {
        QStringList subDirs;
        subDirs.reserve(entries.size());

        // Collect valid subdirectories
        for (const QFileInfo &info : std::as_const(entries)) {
            const QString &subPath = info.absoluteFilePath();
            if (info.isDir() && !info.isSymLink() && !exclusionChecker(subPath)) {
                subDirs << subPath;
            }
        }

        // Emit signal with found subdirectories
        if (!subDirs.isEmpty()) {
            emit subdirectoriesFound(subDirs);
        }
    }
}

void FSMonitorWorker::tryFastDirectoryScan()
{
    // Only allow one fast scan at a time
    if (fastScanInProgress) {
        fmWarning() << "FSMonitorWorker: Fast directory scan already in progress, ignoring request";
        return;
    }

    fmInfo() << "FSMonitorWorker: Starting fast directory scan using SearchEngine";
    fastScanInProgress = true;

    // Create a lambda for the async operation
    // Capture by value to avoid accessing destroyed object
    auto capturedMaxResults = maxResultsCount;
    auto capturedExclusionChecker = exclusionChecker;

    auto scanOperation = [capturedMaxResults, capturedExclusionChecker]() -> QStringList {
        SearchResultList directories;
        auto status = DFMSEARCH::Global::fileNameIndexStatus();

        if (!status.has_value()) {
            return {};
        }

        const QString currentStatus = status.value();
        if (currentStatus == "closed") {
            fmWarning() << "FSMonitorWorker: Cannot use fast directory scan, index status is:" << currentStatus;
            return {};
        }

        QObject holder;
        SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);
        SearchOptions options;
        options.setMaxResults(capturedMaxResults);
        options.setSyncSearchTimeout(120);
        options.setSearchPath(QDir::rootPath());
        options.setSearchMethod(SearchMethod::Indexed);
        FileNameOptionsAPI fileNameOptions(options);
        fileNameOptions.setFileTypes({ Defines::kAnythingDirType });
        engine->setSearchOptions(options);
        SearchQuery query = SearchFactory::createQuery("", SearchQuery::Type::Simple);
        const SearchResultExpected &result = engine->searchSync(query);
        if (result.hasValue() && !result->isEmpty()) {
            directories = std::move(result.value());
        }

        fmInfo() << "FSMonitorWorker: Fast directory scan found" << directories.size() << "directories";
        // Filter excluded directories
        QStringList filteredDirs;
        for (const auto &dir : std::as_const(directories)) {
            const QString &path = dir.path();
            if (!capturedExclusionChecker(path)) {
                filteredDirs << path;
            }
        }

        return filteredDirs;
    };

    // Start async operation
    QFuture<QStringList> future = QtConcurrent::run(scanOperation);
    futureWatcher->setFuture(future);
}

void FSMonitorWorker::handleFastScanResult()
{
    QStringList directories = futureWatcher->result();
    bool success = !directories.isEmpty();

    if (success) {
        // Fast scan succeeded
        fmInfo() << "FSMonitorWorker: Fast directory scan succeeded, found" << directories.size() << "directories";

        // Emit directories in batches to avoid overwhelming the main thread
        const int kBatchSize = 200;
        for (int i = 0; i < directories.size(); i += kBatchSize) {
            QStringList batch = directories.mid(i, kBatchSize);
            emit directoriesBatchToWatch(batch);
            QThread::msleep(100);
        }
    } else {
        // Fast scan failed
        fmInfo() << "FSMonitorWorker: Fast directory scan failed, system will fall back to traditional scanning";
    }

    // Reset flag and signal completion
    fastScanInProgress = false;
    emit fastScanCompleted(success);
}

void FSMonitorWorker::setExclusionChecker(const std::function<bool(const QString &)> &checker)
{
    if (checker) {
        exclusionChecker = checker;
    }
}

void FSMonitorWorker::setMaxFastScanResults(int max)
{
    if (max > 0) {
        maxResultsCount = max;
    }
}

SERVICETEXTINDEX_END_NAMESPACE
