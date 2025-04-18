// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fseventcollector_p.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/utils/protocolutils.h>

#include <QDir>
#include <QFileInfo>

DAEMONPCORE_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

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

void FSEventCollectorPrivate::handleFileCreated(const QString &path, const QString &name)
{
    QString fullPath = normalizePath(path, name);

    // Skip if max event count exceeded
    if (isMaxEventCountExceeded()) {
        return;
    }

    // Logic for handling file creation:
    // 1. If file was previously deleted, remove from deleted list (creation cancels deletion)
    // 2. Otherwise, add to created list
    if (deletedFilesList.contains(fullPath)) {
        deletedFilesList.remove(fullPath);
        logDebug(QString("Removed from deleted list due to re-creation: %1").arg(fullPath));
    } else {
        createdFilesList.insert(fullPath);
        logDebug(QString("Added to created list: %1").arg(fullPath));
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
    } else {
        if (modifiedFilesList.contains(fullPath)) {
            modifiedFilesList.remove(fullPath);
            logDebug(QString("Removed from modified list due to deletion: %1").arg(fullPath));
        }
        deletedFilesList.insert(fullPath);
        logDebug(QString("Added to deleted list: %1").arg(fullPath));
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
        modifiedFilesList.insert(fullPath);
        logDebug(QString("Added to modified list: %1").arg(fullPath));
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
    if (dirPath.isEmpty() || fileName.isEmpty()) {
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

DAEMONPCORE_END_NAMESPACE
