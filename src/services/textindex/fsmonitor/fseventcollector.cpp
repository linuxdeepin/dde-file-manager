// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fseventcollector_p.h"
#include "utils/textindexconfig.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/protocolutils.h>

#include <QDir>
#include <QFileInfo>

DFMBASE_USE_NAMESPACE

SERVICETEXTINDEX_BEGIN_NAMESPACE

FSEventCollectorPrivate::FSEventCollectorPrivate(FSEventCollector *qq, FSMonitor &monitor)
    : q_ptr(qq), fsMonitor(monitor)
{
    // Set up collection timer
    collectionTimer.setSingleShot(true);
    QObject::connect(&collectionTimer, &QTimer::timeout, qq, [this]() {
        flushCollectedEvents();

        // Restart timer if still active
        if (active) {
            collectionTimer.start(collectionIntervalMs);
        }
    });
}

FSEventCollectorPrivate::~FSEventCollectorPrivate()
{
    stopCollecting();
}

bool FSEventCollectorPrivate::init(const QString &rootPath)
{
    this->rootPath = QDir(rootPath).absolutePath();

    // Initialize the underlying FSMonitor with the same root path
    if (!fsMonitor.initialize(rootPath)) {
        logError(QString("Failed to initialize FSMonitor with root path: %1").arg(rootPath));
        return false;
    }

    logDebug(QString("FSEventCollector initialized with root path: %1").arg(this->rootPath));
    return true;
}

bool FSEventCollectorPrivate::startCollecting()
{
    if (active) {
        logDebug("Event collection already active, ignoring start request");
        return true;
    }

    // Connect to FSMonitor signals
    QObject::connect(&fsMonitor, &FSMonitor::fileCreated,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleFileCreated(path, name);
                     });

    QObject::connect(&fsMonitor, &FSMonitor::fileDeleted,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleFileDeleted(path, name);
                     });

    QObject::connect(&fsMonitor, &FSMonitor::fileModified,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleFileModified(path, name);
                     });

    QObject::connect(&fsMonitor, &FSMonitor::fileMoved,
                     q_ptr, [this](const QString &fromPath, const QString &fromName, const QString &toPath, const QString &toName) {
                         handleFileMoved(fromPath, fromName, toPath, toName);
                     });

    QObject::connect(&fsMonitor, &FSMonitor::directoryCreated,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleDirectoryCreated(path, name);
                     });

    QObject::connect(&fsMonitor, &FSMonitor::directoryDeleted,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleDirectoryDeleted(path, name);
                     });

    QObject::connect(&fsMonitor, &FSMonitor::directoryMoved,
                     q_ptr, [this](const QString &fromPath, const QString &fromName, const QString &toPath, const QString &toName) {
                         handleDirectoryMoved(fromPath, fromName, toPath, toName);
                     });

    // Clear existing events
    createdFilesList.clear();
    deletedFilesList.clear();
    modifiedFilesList.clear();

    // Start the FSMonitor
    if (!fsMonitor.start()) {
        logError("Failed to start FSMonitor");
        return false;
    }

    // Start collection timer
    active = true;
    collectionTimer.start(collectionIntervalMs);

    logDebug(QString("Started event collection with interval: %1 seconds")
                     .arg(collectionIntervalMs / 1000));

    return true;
}

void FSEventCollectorPrivate::stopCollecting()
{
    if (!active) {
        return;
    }

    active = false;

    // Stop the timer
    collectionTimer.stop();

    // Disconnect from FSMonitor signals
    QObject::disconnect(&fsMonitor, nullptr, q_ptr, nullptr);

    // Stop the FSMonitor
    fsMonitor.stop();

    // Clear collected events

    createdFilesList.clear();
    deletedFilesList.clear();
    modifiedFilesList.clear();

    logDebug("Stopped event collection");
}

bool FSEventCollectorPrivate::shouldIndexFile(const QString &path) const
{
    if (path.isEmpty())
        return false;

    // For deleted files, don't attempt to check file system since it's gone
    if (deletedFilesList.contains(path))
        return true;

    // Always track directories regardless of extension
    if (isDirectory(path))
        return true;

    // Get file suffix for extension check
    QFileInfo fileInfo(path);
    QString suffix = fileInfo.suffix();

    // Maybe it's a deleted folder ?
    if (suffix.isEmpty() && !fileInfo.exists())
        return true;

    // Check if extension is supported for content search
    bool supported = TextIndexConfig::instance().supportedFileExtensions().contains(suffix);
    if (!supported)
        logDebug(QString("Skipping file with unsupported extension: %1 (suffix: %2)").arg(path, suffix));

    return supported;
}

void FSEventCollectorPrivate::handleFileCreated(const QString &path, const QString &name)
{
    QString fullPath = normalizePath(path, name);

    // Skip if max event count exceeded
    if (isMaxEventCountExceeded()) {
        return;
    }

    // Logic for handling file creation:
    // 1. If file was previously deleted, remove from deleted list AND add to created list
    //    (re-creation needs reindexing as content might have changed)
    // 2. Otherwise, add to created list
    if (deletedFilesList.contains(fullPath)) {
        deletedFilesList.remove(fullPath);

        // Add to created list to ensure reindexing
        if (shouldIndexFile(fullPath)) {
            createdFilesList.insert(fullPath);
            logDebug(QString("File was deleted and recreated, adding to created list: %1").arg(fullPath));
        }
    } else {
        // Check if this file is under a directory that's already in the created list
        if (!isChildOfAnyPath(fullPath, createdFilesList)) {
            // Only insert if file has supported extension or is a directory
            if (shouldIndexFile(fullPath)) {
                createdFilesList.insert(fullPath);
                logDebug(QString("Added to created list: %1").arg(fullPath));

                // If this is a directory, remove any files in the list that are under this directory
                if (isDirectory(fullPath)) {
                    removeRedundantEntries(createdFilesList);
                }
            }
        } else {
            logDebug(QString("Skipped adding to created list (parent directory already added): %1").arg(fullPath));
        }
    }

    // Check if max event count exceeded after adding
    if (isMaxEventCountExceeded()) {
        flushCollectedEvents();
        Q_EMIT q_ptr->maxEventCountReached(maxEvents);
    }
}

void FSEventCollectorPrivate::handleFileDeleted(const QString &path, const QString &name)
{
    QString fullPath = normalizePath(path, name);

    // Skip if max event count exceeded
    if (isMaxEventCountExceeded()) {
        return;
    }

    // Logic for handling file deletion:
    // 1. If file was previously created, remove from created list (deletion cancels creation)
    // 2. If file was previously modified, remove from modified list (deletion supersedes modification)
    // 3. Otherwise, add to deleted list
    if (createdFilesList.contains(fullPath)) {
        createdFilesList.remove(fullPath);
        logDebug(QString("Removed from created list due to deletion: %1").arg(fullPath));
        if (shouldIndexFile(fullPath)) {
            deletedFilesList.insert(fullPath);
            logDebug(QString("Added to deleted list: %1").arg(fullPath));
        }
    } else {
        if (modifiedFilesList.contains(fullPath)) {
            modifiedFilesList.remove(fullPath);
            logDebug(QString("Removed from modified list due to deletion: %1").arg(fullPath));
        }

        if (shouldIndexFile(fullPath)) {
            deletedFilesList.insert(fullPath);
            logDebug(QString("Added to deleted list: %1").arg(fullPath));
        } else {
            logDebug(QString("Skipped adding to deleted list (parent directory already added): %1").arg(fullPath));
        }
    }

    // Check if max event count exceeded after adding
    if (isMaxEventCountExceeded()) {
        flushCollectedEvents();
        Q_EMIT q_ptr->maxEventCountReached(maxEvents);
    }
}

void FSEventCollectorPrivate::handleFileModified(const QString &path, const QString &name)
{
    QString fullPath = normalizePath(path, name);

    // Skip if max event count exceeded
    if (isMaxEventCountExceeded()) {
        return;
    }

    // Logic for handling file modification:
    // 1. If file was previously created, don't add to modified (creation includes latest content)
    // 2. If file is in deleted list, ignore (deleted files can't be modified)
    // 3. Otherwise, add to modified list
    if (createdFilesList.contains(fullPath) || deletedFilesList.contains(fullPath)) {
        // No need to track modifications for newly created or deleted files
        logDebug(QString("Ignored modification for created/deleted file: %1").arg(fullPath));
    } else {
        // For modified files, we only care about actual files, not directories
        // So we don't need to check for parent directories or redundant entries
        if (!isDirectory(fullPath) && !isChildOfAnyPath(fullPath, createdFilesList) && !isChildOfAnyPath(fullPath, deletedFilesList)) {
            // Only insert if file has supported extension
            if (shouldIndexFile(fullPath)) {
                modifiedFilesList.insert(fullPath);
                logDebug(QString("Added to modified list: %1").arg(fullPath));
            }
        } else {
            logDebug(QString("Skipped adding to modified list (directory or parent directory already in lists): %1").arg(fullPath));
        }
    }

    // Check if max event count exceeded after adding
    if (isMaxEventCountExceeded()) {
        flushCollectedEvents();
        Q_EMIT q_ptr->maxEventCountReached(maxEvents);
    }
}

void FSEventCollectorPrivate::handleFileMoved(const QString &fromPath, const QString &fromName,
                                              const QString &toPath, const QString &toName)
{
    // Handle file moves as deletion from source and creation at destination

    // Special case: Move to outside monitored directory
    if (toPath.isEmpty() && toName.isEmpty()) {
        handleFileDeleted(fromPath, fromName);
        return;
    }

    // Special case: Move from outside monitored directory
    if (fromPath.isEmpty() && fromName.isEmpty()) {
        handleFileCreated(toPath, toName);
        return;
    }

    // Regular move within monitored directories
    handleFileDeleted(fromPath, fromName);
    handleFileCreated(toPath, toName);
}

void FSEventCollectorPrivate::handleDirectoryCreated(const QString &path, const QString &name)
{
    handleFileCreated(path, name);
}

void FSEventCollectorPrivate::handleDirectoryDeleted(const QString &path, const QString &name)
{
    handleFileDeleted(path, name);
}

void FSEventCollectorPrivate::handleDirectoryMoved(const QString &fromPath, const QString &fromName,
                                                   const QString &toPath, const QString &toName)
{
    handleFileMoved(fromPath, fromName, toPath, toName);
}

void FSEventCollectorPrivate::flushCollectedEvents()
{
    // First, clean up redundant entries in all lists
    cleanupRedundantEntries();

    // Prepare the lists of events to emit
    QStringList created = createdFilesList.values();
    QStringList deleted = deletedFilesList.values();
    QStringList modified = modifiedFilesList.values();

    // Clear the internal sets for next collection period
    createdFilesList.clear();
    deletedFilesList.clear();
    modifiedFilesList.clear();

    // Log statistics
    logDebug(QString("Flushing events - Created: %1, Deleted: %2, Modified: %3")
                     .arg(created.size())
                     .arg(deleted.size())
                     .arg(modified.size()));

    // Emit signals with collected events (only if not empty)
    if (!created.isEmpty()) {
        Q_EMIT q_ptr->filesCreated(created);
    }

    if (!deleted.isEmpty()) {
        Q_EMIT q_ptr->filesDeleted(deleted);
    }

    if (!modified.isEmpty()) {
        Q_EMIT q_ptr->filesModified(modified);
    }

    Q_EMIT q_ptr->flushFinished();
}

void FSEventCollectorPrivate::removeRedundantEntries(QSet<QString> &filesList)
{
    QSet<QString> directoryPaths;
    QSet<QString> filePaths;

    // First, separate directories from files
    for (const QString &path : filesList) {
        if (isDirectory(path)) {
            directoryPaths.insert(path);
        } else {
            filePaths.insert(path);
        }
    }

    // Remove any files that are under any of the directories
    QSet<QString> redundantPaths;
    for (const QString &filePath : filePaths) {
        if (isChildOfAnyPath(filePath, directoryPaths)) {
            redundantPaths.insert(filePath);
        }
    }

    // Remove redundant paths from the original list
    for (const QString &path : redundantPaths) {
        filesList.remove(path);
        logDebug(QString("Removed redundant entry (parent directory exists in list): %1").arg(path));
    }
}

bool FSEventCollectorPrivate::isChildOfAnyPath(const QString &path, const QSet<QString> &pathSet) const
{
    // Quick check for empty set
    if (pathSet.isEmpty() || path.isEmpty()) {
        return false;
    }

    // For each potential parent directory in the set
    for (const QString &potentialParent : pathSet) {
        // Skip if the potential parent is not a directory
        if (!isDirectory(potentialParent)) {
            continue;
        }

        // Check if path starts with potentialParent + "/"
        // First, normalize both paths to ensure consistent comparison
        QString normalizedPath = QDir::cleanPath(path);
        QString normalizedParent = QDir::cleanPath(potentialParent);

        // Make sure the parent path ends with a separator
        if (!normalizedParent.endsWith('/')) {
            normalizedParent += '/';
        }

        // Path is a child if it starts with the parent path (including the trailing slash)
        if (normalizedPath.startsWith(normalizedParent)) {
            return true;
        }
    }

    return false;
}

bool FSEventCollectorPrivate::isDirectory(const QString &path) const
{
    QFileInfo fileInfo(path);
    return fileInfo.isDir() && !fileInfo.isSymLink();
}

void FSEventCollectorPrivate::cleanupRedundantEntries()
{
    // Clean up each list separately
    removeRedundantEntries(createdFilesList);
    removeRedundantEntries(deletedFilesList);

    // For the modified list, we need to remove entries that:
    // 1. Are under directories in the created list (creation supersedes modification)
    // 2. Are under directories in the deleted list (deletion supersedes modification)

    // First, get all directory paths from created and deleted lists
    QSet<QString> allDirectories;
    for (const QString &path : std::as_const(createdFilesList)) {
        if (isDirectory(path)) {
            allDirectories.insert(path);
        }
    }
    for (const QString &path : std::as_const(deletedFilesList)) {
        if (isDirectory(path)) {
            allDirectories.insert(path);
        }
    }

    // Remove any modified files that are under these directories
    QSet<QString> redundantModified;
    for (const QString &modifiedPath : std::as_const(modifiedFilesList)) {
        if (isChildOfAnyPath(modifiedPath, allDirectories)) {
            redundantModified.insert(modifiedPath);
            logDebug(QString("Removed redundant modified entry (parent directory in created/deleted lists): %1").arg(modifiedPath));
        }
    }

    // Remove the redundant entries
    for (const QString &path : redundantModified) {
        modifiedFilesList.remove(path);
    }
}

bool FSEventCollectorPrivate::isMaxEventCountExceeded() const
{
    int total = createdFilesList.size() + deletedFilesList.size() + modifiedFilesList.size();
    return total >= maxEvents;
}

void FSEventCollectorPrivate::logDebug(const QString &message) const
{
    fmDebug() << "FSEventCollector:" << message;
}

void FSEventCollectorPrivate::logError(const QString &message) const
{
    fmWarning() << "FSEventCollector ERROR:" << message;
    Q_EMIT q_ptr->errorOccurred(message);
}

QString FSEventCollectorPrivate::normalizePath(const QString &dirPath, const QString &fileName) const
{
    if (dirPath.isEmpty()) {
        return QString();
    }

    // Combine path components correctly handling trailing slash
    return buildPath(dirPath, fileName);
}

QString FSEventCollectorPrivate::buildPath(const QString &dirPath, const QString &fileName) const
{
    QDir dir(dirPath);
    return dir.absoluteFilePath(fileName);
}

// FSEventCollector implementation

// Constructor with explicitly provided FSMonitor
FSEventCollector::FSEventCollector(FSMonitor &monitor, QObject *parent)
    : QObject(parent), d_ptr(new FSEventCollectorPrivate(this, monitor))
{
}

// Constructor with default FSMonitor (uses singleton instance)
FSEventCollector::FSEventCollector(QObject *parent)
    : QObject(parent), d_ptr(new FSEventCollectorPrivate(this, FSMonitor::instance()))
{
}

FSEventCollector::~FSEventCollector()
{
    // The QScopedPointer will automatically call the destructor
    // of FSEventCollectorPrivate, which stops collecting
}

bool FSEventCollector::initialize(const QString &rootPath)
{
    Q_D(FSEventCollector);
    return d->init(rootPath);
}

bool FSEventCollector::start()
{
    Q_D(FSEventCollector);
    return d->startCollecting();
}

void FSEventCollector::stop()
{
    Q_D(FSEventCollector);
    d->stopCollecting();
}

bool FSEventCollector::isActive() const
{
    Q_D(const FSEventCollector);
    return d->active;
}

void FSEventCollector::setCollectionInterval(int seconds)
{
    Q_D(FSEventCollector);

    // Validate input
    if (seconds <= 0) {
        d->logError(QString("Invalid collection interval: %1 seconds. Must be positive.").arg(seconds));
        return;
    }

    // Convert to milliseconds
    d->collectionIntervalMs = seconds * 1000;

    // Restart timer if active
    if (d->active && d->collectionTimer.isActive()) {
        d->collectionTimer.stop();
        d->collectionTimer.start(d->collectionIntervalMs);
    }

    d->logDebug(QString("Collection interval set to %1 seconds").arg(seconds));
}

int FSEventCollector::collectionInterval() const
{
    Q_D(const FSEventCollector);
    return d->collectionIntervalMs / 1000;
}

void FSEventCollector::setMaxEventCount(int count)
{
    Q_D(FSEventCollector);

    // Validate input
    if (count <= 0) {
        d->logError(QString("Invalid max event count: %1. Must be positive.").arg(count));
        return;
    }

    d->maxEvents = count;
    d->logDebug(QString("Max event count set to %1").arg(count));

    // Check if current count exceeds new max
    if (d->isMaxEventCountExceeded()) {
        d->flushCollectedEvents();
        Q_EMIT maxEventCountReached(count);
    }
}

int FSEventCollector::maxEventCount() const
{
    Q_D(const FSEventCollector);
    return d->maxEvents;
}

void FSEventCollector::flushEvents()
{
    Q_D(FSEventCollector);
    d->flushCollectedEvents();
}

void FSEventCollector::clearEvents()
{
    Q_D(FSEventCollector);

    d->createdFilesList.clear();
    d->deletedFilesList.clear();
    d->modifiedFilesList.clear();

    d->logDebug("Cleared all collected events");
}

QStringList FSEventCollector::createdFiles() const
{
    Q_D(const FSEventCollector);

    return d->createdFilesList.values();
}

QStringList FSEventCollector::deletedFiles() const
{
    Q_D(const FSEventCollector);

    return d->deletedFilesList.values();
}

QStringList FSEventCollector::modifiedFiles() const
{
    Q_D(const FSEventCollector);

    return d->modifiedFilesList.values();
}

int FSEventCollector::createdFilesCount() const
{
    Q_D(const FSEventCollector);

    return d->createdFilesList.size();
}

int FSEventCollector::deletedFilesCount() const
{
    Q_D(const FSEventCollector);

    return d->deletedFilesList.size();
}

int FSEventCollector::modifiedFilesCount() const
{
    Q_D(const FSEventCollector);

    return d->modifiedFilesList.size();
}

int FSEventCollector::totalEventsCount() const
{
    Q_D(const FSEventCollector);

    return d->createdFilesList.size() + d->deletedFilesList.size() + d->modifiedFilesList.size();
}

SERVICETEXTINDEX_END_NAMESPACE
