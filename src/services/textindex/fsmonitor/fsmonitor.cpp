// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fsmonitor_p.h"
#include "utils/textindexconfig.h"

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-search/dsearch_global.h>

#include <DFileSystemWatcher>

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QUrl>
#include <QTimer>
#include <QCoreApplication>

DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE

SERVICETEXTINDEX_BEGIN_NAMESPACE

// Example:
// void testMonitor()
// {
//     FSMonitor::instance().setMaxResourceUsage(0.6);

//     connect(&FSMonitor::instance(), &FSMonitor::fileCreated, this, [](const QString &path, const QString &name) {
//         fmDebug() << "File created:" << path << "/" << name;
//     });
//     connect(&FSMonitor::instance(), &FSMonitor::fileDeleted, this, [](const QString &path, const QString &name) {
//         fmDebug() << "File deleted:" << path << "/" << name;
//     });
//     connect(&FSMonitor::instance(), &FSMonitor::fileModified, this, [](const QString &path, const QString &name) {
//         fmDebug() << "File modified:" << path << "/" << name;
//     });
//     connect(&FSMonitor::instance(), &FSMonitor::fileMoved, this, [](const QString &fromPath, const QString &fromName, const QString &toPath, const QString &toName) {
//         fmDebug() << "File moved:"
//                   << fromPath << "/" << fromName
//                   << "->"
//                   << toPath << "/" << toName;
//     });
//     connect(&FSMonitor::instance(), &FSMonitor::directoryCreated, this, [](const QString &path, const QString &name) {
//         fmDebug() << "File created:" << path << "/" << name;
//     });
//     connect(&FSMonitor::instance(), &FSMonitor::directoryDeleted, this, [](const QString &path, const QString &name) {
//         fmDebug() << "File deleted:" << path << "/" << name;
//     });
//     connect(&FSMonitor::instance(), &FSMonitor::directoryMoved, this, [](const QString &fromPath, const QString &fromName, const QString &toPath, const QString &toName) {
//         fmDebug() << "File moved:"
//                   << fromPath << "/" << fromName
//                   << "->"
//                   << toPath << "/" << toName;
//     });

//     if (FSMonitor::instance().initialize("/home/zhangs/Videos")) {
//         if (FSMonitor::instance().start()) {
//             fmDebug() << "File monitoring started successfully";
//             fmDebug() << "Max watches available:" << FSMonitor::instance().maxAvailableWatchCount();
//             fmDebug() << "Current usage limit:" << FSMonitor::instance().maxResourceUsage() * 100 << "%";
//         } else {
//             fmWarning() << "Failed to start file monitoring";
//         }
//     } else {
//         fmWarning() << "Failed to initialize FSMonitor";
//     }
// }

FSMonitorPrivate::FSMonitorPrivate(FSMonitor *qq)
    : q_ptr(qq)
{
    // Setup worker thread
    setupWorkerThread();
}

FSMonitorPrivate::~FSMonitorPrivate()
{
    stopMonitoring();

    // Clean up worker thread
    if (workerThread.isRunning()) {
        workerThread.quit();
        workerThread.wait();
    }
}

bool FSMonitorPrivate::init(const QStringList &rootPaths)
{
    // Initialize with root paths and create the watcher
    this->rootPaths.clear();
    for (const QString &path : rootPaths) {
        QString absPath = QDir(path).absolutePath();
        if (QDir(absPath).exists()) {
            this->rootPaths.append(absPath);
        } else {
            fmWarning() << "FSMonitor: Root path does not exist:" << absPath;
        }
    }

    if (this->rootPaths.isEmpty()) {
        fmWarning() << "FSMonitor: No valid root paths provided for initialization";
        return false;
    }

    watcher.reset(new DFileSystemWatcher());

    // Setup watcher connections
    setupWatcherConnections();

    // Use static factory method to create configured blacklist matcher
    excludeMatcher = PathExcludeMatcher::createForIndex();
    fmDebug() << "FSMonitor: Initialized with" << excludeMatcher.patternCount()
              << "blacklist patterns";

    // Configure worker with exclusion logic
    worker->setExclusionChecker([this](const QString &path) {
        return shouldExcludePath(path);
    });
    worker->setMaxFastScanResults(getMaxUserWatches());

    fmInfo() << "FSMonitor: Initialized successfully with" << this->rootPaths.size() << "root paths";
    return true;
}

bool FSMonitorPrivate::startMonitoring()
{
    if (active) {
        fmInfo() << "FSMonitor: Monitoring already active, ignoring start request";
        return true;
    }

    // Determine system limits for inotify watches
    maxWatches = getMaxUserWatches();
    if (maxWatches <= 0) {
        fmWarning() << "FSMonitor: Failed to determine system max watches, using default of 8192";
        maxWatches = 8192;
    }

    // Start monitoring
    active = true;
    watchedDirectories.clear();
    resourceLimitReached = false;   // Reset resource limit flag

    // Start worker thread
    if (!workerThread.isRunning()) {
        workerThread.start();
    }

    // Try fast directory scan first if enabled
    if (useFastScan) {
        fmInfo() << "FSMonitor: Attempting fast directory scan";
        QMetaObject::invokeMethod(worker, "tryFastDirectoryScan",
                                  Qt::QueuedConnection);
    } else {
        travelRootDirectories();
    }

    fmInfo() << "FSMonitor: Started monitoring with max watches:" << maxWatches
             << "usage limit:" << (maxUsagePercentage * 100) << "%";

    return true;
}

void FSMonitorPrivate::stopMonitoring()
{
    if (!active) {
        return;
    }

    active = false;

    // Clear all watched directories
    if (!watchedDirectories.isEmpty() && watcher) {
        watcher->removePaths(watchedDirectories.values());
        watchedDirectories.clear();
    }

    fmInfo() << "FSMonitor: Stopped all monitoring";
}

void FSMonitorPrivate::travelRootDirectories()
{
    // Process the root directories using traditional method
    // If fast scan succeeds, this will complement any directories
    // that might have been missed. If fast scan fails, this ensures
    // we still monitor everything.
    for (const QString &dir : std::as_const(rootPaths)) {
        QMetaObject::invokeMethod(worker, "processDirectory",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, dir));
    }
}

void FSMonitorPrivate::setupWorkerThread()
{
    // Create worker and move to thread
    worker = new FSMonitorWorker();
    worker->moveToThread(&workerThread);

    // Connect thread finished signal to clean up worker
    QObject::connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

    // Connect worker signals
    QObject::connect(
            worker, &FSMonitorWorker::directoryToWatch,
            q_ptr, [this](const QString &path) {
                // Skip processing if resource limit has been reached
                if (!resourceLimitReached) {
                    addWatchForDirectory(path);
                }
            },
            Qt::QueuedConnection);

    QObject::connect(
            worker, &FSMonitorWorker::subdirectoriesFound,
            q_ptr, [this](const QStringList &directories) {
                // Skip processing if resource limit has been reached or monitoring is not active
                if (!active || resourceLimitReached) {
                    return;
                }

                // Process each subdirectory
                for (const QString &dir : directories) {
                    if (active && !resourceLimitReached) {
                        QMetaObject::invokeMethod(worker, "processDirectory",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(QString, dir));
                    } else {
                        // Stop processing if limit reached during iteration
                        break;
                    }
                }
            },
            Qt::QueuedConnection);

    // Connect fast scan related signals
    QObject::connect(
            worker, &FSMonitorWorker::fastScanCompleted,
            q_ptr, [this](bool success) {
                handleFastScanCompleted(success);
            },
            Qt::QueuedConnection);

    QObject::connect(
            worker, &FSMonitorWorker::directoriesBatchToWatch,
            q_ptr, [this](const QStringList &paths) {
                // Skip processing if resource limit has been reached
                if (!resourceLimitReached) {
                    handleDirectoriesBatch(paths);
                }
            },
            Qt::QueuedConnection);
}

void FSMonitorPrivate::addDirectoryRecursively(const QString &path)
{
    if (!active || path.isEmpty() || resourceLimitReached) {
        return;
    }

    // Queue the directory for processing
    QMetaObject::invokeMethod(worker, "processDirectory",
                              Qt::QueuedConnection,
                              Q_ARG(QString, path));
}

int FSMonitorPrivate::getMaxUserWatches() const
{
    int maxWatches = -1;

    // Try to read the system limit from /proc
    QFile file("/proc/sys/fs/inotify/max_user_watches");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        bool ok = false;
        maxWatches = in.readLine().trimmed().toInt(&ok);
        if (!ok) {
            maxWatches = -1;
        }
        file.close();
    }

    return maxWatches;
}

bool FSMonitorPrivate::shouldExcludePath(const QString &path) const
{
    // Skip empty paths
    if (path.isEmpty()) {
        return true;
    }

    // Check if path is a symlink
    if (isSymbolicLink(path)) {
        fmDebug() << "FSMonitor: Excluding symbolic link:" << path;
        return true;
    }

    // Skip hidden files/dirs if hidden files aren't enabled in settings
    if (!showHidden()) {
        QFileInfo fileInfo(path);
        if (fileInfo.fileName().startsWith('.')) {
            fmDebug() << "FSMonitor: Excluding hidden file/directory:" << path;
            return true;
        }

        // Check if any parent directory is hidden
        if (DFMSEARCH::Global::isHiddenPathOrInHiddenDir(fileInfo.absoluteFilePath())) {
            fmDebug() << "FSMonitor: Excluding file in hidden directory:" << path;
            return true;
        }
    }

    // Calculate absolute path safely
    QDir dir(path);
    const QString absolutePath = dir.exists() ? dir.absolutePath() : path;

    // Check against blacklisted paths using PathExcludeMatcher
    if (excludeMatcher.shouldExclude(absolutePath)) {
        return true;
    }

    // 以下判断严重影响性能，如无问题反馈则屏蔽
    // // Check if path is on external mount
    // // Note: Block devices allowed to be mounted via udisks
    // if (!DevProxyMng->isFileOfExternalBlockMounts(path) && isExternalMount(path)) {
    //     fmDebug() << "FSMonitor: Excluding external mount:" << path;
    //     return true;
    // }

    return false;
}

bool FSMonitorPrivate::isSymbolicLink(const QString &path) const
{
    // Skip empty paths
    if (path.isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(path);
    return fileInfo.isSymLink();
}

bool FSMonitorPrivate::showHidden() const
{
    return TextIndexConfig::instance().indexHiddenFiles();
}

bool FSMonitorPrivate::isExternalMount(const QString &path) const
{
    // Skip empty paths
    if (path.isEmpty()) {
        return false;
    }

    QStorageInfo storage(path);

    // Skip if not mounted or if it's a network filesystem
    if (!storage.isValid() || !storage.isReady()) {
        return false;
    }

    // Check for network filesystems
    const QString fsType = storage.fileSystemType().toLower();

    // Reject all FUSE-based filesystems
    if (fsType.startsWith("fuse")) {
        return true;
    }

    // Check for other known network filesystems
    // TODO: add to DConfig
    static const QStringList kNetworkFsTypes = {
        "nfs", "nfs4", "cifs", "smb", "smb2", "smbfs", "webdav",
        "ceph", "glusterfs", "moosefs", "lustre", "overlay", "aufs", "9p",
        "sftp", "curlftpfs", "davfs"
    };

    if (kNetworkFsTypes.contains(fsType.toLower())) {
        return true;
    }

    // Check for removable media
    if (!ProtocolUtils::isLocalFile(QUrl::fromLocalFile(path))) {
        return true;
    }

    return false;
}

bool FSMonitorPrivate::addWatchForDirectory(const QString &path)
{
    // Skip if path is empty or should exclude
    if (path.isEmpty() || shouldExcludePath(path)) {
        return false;
    }

    if (watchedDirectories.contains(path)) {
        return true;   // Already watching
    }

    // Check if we're within watch limits
    if (!isWithinWatchLimit()) {
        // Only emit warning and signal once when limit is first reached
        if (!resourceLimitReached) {
            fmWarning() << "FSMonitor: Watch limit reached (" << watchedDirectories.size()
                        << "/" << maxWatches << "), stopping further directory monitoring";
            resourceLimitReached = true;

            // Notify about the resource limit
            Q_EMIT q_ptr->resourceLimitReached(watchedDirectories.size(), maxWatches);
        }
        return false;
    }

    // Add to watcher
    if (watcher->addPath(path)) {
        watchedDirectories.insert(path);
        //   fmDebug() << "FSMonitor: Added watch for directory:" << path;
        return true;
    }

    fmWarning() << "FSMonitor: Failed to add watch for directory:" << path;
    return false;
}

void FSMonitorPrivate::removeWatchForDirectory(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    if (watchedDirectories.contains(path)) {
        watcher->removePath(path);
        watchedDirectories.remove(path);
        fmDebug() << "FSMonitor: Removed watch for directory:" << path;
    }
}

bool FSMonitorPrivate::isWithinWatchLimit() const
{
    if (maxWatches <= 0) {
        return true;   // No limit determined, assume it's fine
    }

    int maxAllowed = qFloor(maxWatches * maxUsagePercentage);
    return watchedDirectories.size() < maxAllowed;
}

void FSMonitorPrivate::setupWatcherConnections()
{
    // Connect file events from DFileSystemWatcher
    QObject::connect(watcher.data(), &DFileSystemWatcher::fileCreated,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleFileCreated(path, name);
                     });

    QObject::connect(watcher.data(), &DFileSystemWatcher::fileDeleted,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleFileDeleted(path, name);
                     });

    QObject::connect(watcher.data(), &DFileSystemWatcher::fileModified,
                     q_ptr, [this](const QString &path, const QString &name) {
                         handleFileModified(path, name);
                     });

    QObject::connect(watcher.data(), &DFileSystemWatcher::fileMoved,
                     q_ptr, [this](const QString &fromPath, const QString &fromName, const QString &toPath, const QString &toName) {
                         handleFileMoved(fromPath, fromName, toPath, toName);
                     });
}

void FSMonitorPrivate::handleFileCreated(const QString &path, const QString &name)
{
    if (!active || path.isEmpty()) {
        return;
    }

    // Skip hidden files if needed
    if (!showHidden() && name.startsWith('.')) {
        return;
    }

    QString fullPath = QDir(path).absoluteFilePath(name);

    if (isDirectory(path, name)) {
        // It's a directory, add to watch and emit signal
        Q_EMIT q_ptr->directoryCreated(path, name);

        // Add new directory to watch recursively
        if (!isSymbolicLink(fullPath) && !shouldExcludePath(fullPath)) {
            addDirectoryRecursively(fullPath);
        }
    } else {
        // It's a file, emit signal
        Q_EMIT q_ptr->fileCreated(path, name);
    }
}

void FSMonitorPrivate::handleFileDeleted(const QString &path, const QString &name)
{
    if (!active || path.isEmpty()) {
        return;
    }

    // Skip hidden files if needed
    if (!showHidden() && name.startsWith('.')) {
        return;
    }

    QString fullPath = QDir(path).absoluteFilePath(name);

    if (watchedDirectories.contains(fullPath)) {
        // It was a watched directory
        Q_EMIT q_ptr->directoryDeleted(path, name);

        // Remove from watch
        watcher->removePath(fullPath);
        watchedDirectories.remove(fullPath);
    } else {
        // Regular file deleted
        if (!name.isEmpty()) {
            Q_EMIT q_ptr->fileDeleted(path, name);
        }
    }
}

void FSMonitorPrivate::handleFileModified(const QString &path, const QString &name)
{
    if (!active || path.isEmpty()) {
        return;
    }

    // Skip hidden files if needed
    if (!showHidden() && name.startsWith('.')) {
        return;
    }

    Q_EMIT q_ptr->fileModified(path, name);
}

void FSMonitorPrivate::handleFileMoved(const QString &fromPath, const QString &fromName,
                                       const QString &toPath, const QString &toName)
{
    if (!active) {
        return;
    }

    // Skip hidden files if needed
    if (!showHidden() && toName.startsWith('.')) {
        return;
    }

    QString fromFullPath = QDir(fromPath).absoluteFilePath(fromName);
    QString toFullPath = QDir(toPath).absoluteFilePath(toName);

    if (watchedDirectories.contains(fromFullPath)) {
        // It was a watched directory that was moved
        fmDebug() << "FSMonitor: Directory moved:" << fromPath << "/" << fromName
                  << "->" << toPath << "/" << toName;

        Q_EMIT q_ptr->directoryMoved(fromPath, fromName, toPath, toName);

        // Update directory watches
        watcher->removePath(fromFullPath);
        watchedDirectories.remove(fromFullPath);

        // Add the destination directory to watch
        if (!toPath.isEmpty() && !isSymbolicLink(toFullPath) && !shouldExcludePath(toFullPath)) {
            addDirectoryRecursively(toFullPath);
        }
    } else {
        // Regular file moved
        fmDebug() << "FSMonitor: File moved:" << fromPath << "/" << fromName
                  << "->" << toPath << "/" << toName;

        Q_EMIT q_ptr->fileMoved(fromPath, fromName, toPath, toName);
    }
}

bool FSMonitorPrivate::isDirectory(const QString &path, const QString &name) const
{
    if (path.isEmpty()) {
        return false;
    }

    return QFileInfo(QDir(path).absoluteFilePath(name)).isDir();
}

void FSMonitorPrivate::handleFastScanCompleted(bool success)
{
    if (success) {
        fmDebug() << "FSMonitor: Fast directory scan completed successfully";
    } else {
        fmWarning() << "FSMonitor: Fast directory scan failed, continuing with traditional scan";
        travelRootDirectories();
    }
}

void FSMonitorPrivate::handleDirectoriesBatch(const QStringList &paths)
{
    if (paths.isEmpty()) {
        return;
    }

    fmDebug() << "FSMonitor: Received batch of" << paths.size() << "directories to watch";

    int addedCount = 0;
    int skipCount = 0;
    int failCount = 0;

    // Process this batch directly, but stop early if resource limit is reached
    for (const QString &path : paths) {
        // Check if resource limit has been reached - early exit to avoid processing remaining paths
        if (resourceLimitReached || !isWithinWatchLimit()) {
            if (!resourceLimitReached) {
                fmWarning() << "FSMonitor: Watch limit reached (" << watchedDirectories.size()
                            << "/" << maxWatches << "), stopping batch processing";
                resourceLimitReached = true;
                Q_EMIT q_ptr->resourceLimitReached(watchedDirectories.size(), maxWatches);
            }
            break;
        }

        if (!watchedDirectories.contains(path) && !shouldExcludePath(path)) {
            qApp->processEvents();
            // Add each path individually to avoid blocking from addPaths
            if (watcher->addPath(path)) {
                watchedDirectories.insert(path);
                addedCount++;
            } else {
                fmWarning() << "FSMonitor: Failed to add directory watch:" << path;
                failCount++;
            }
        } else {
            skipCount++;
        }
    }

    fmDebug() << "FSMonitor: Batch processing complete - added:" << addedCount
              << "skipped:" << skipCount
              << "failed:" << failCount
              << "total watching:" << watchedDirectories.size();
}

// ========== FSMonitor implementation ==========

FSMonitor &FSMonitor::instance()
{
    static FSMonitor ins;
    return ins;
}

FSMonitor::FSMonitor(QObject *parent)
    : QObject(parent), d_ptr(new FSMonitorPrivate(this))
{
}

FSMonitor::~FSMonitor()
{
    stop();
}

bool FSMonitor::initialize(const QStringList &rootPaths)
{
    Q_D(FSMonitor);
    return d->init(rootPaths);
}

bool FSMonitor::start()
{
    Q_D(FSMonitor);
    return d->startMonitoring();
}

void FSMonitor::stop()
{
    Q_D(FSMonitor);
    d->stopMonitoring();
}

bool FSMonitor::isActive() const
{
    Q_D(const FSMonitor);
    return d->active;
}

void FSMonitor::addBlacklistedPath(const QString &path)
{
    Q_D(FSMonitor);
    d->excludeMatcher.addPattern(path);
}

void FSMonitor::addBlacklistedPaths(const QStringList &paths)
{
    Q_D(FSMonitor);
    d->excludeMatcher.addPatterns(paths);
}

void FSMonitor::removeBlacklistedPath(const QString &path)
{
    Q_D(FSMonitor);
    d->excludeMatcher.removePattern(path);
}

QStringList FSMonitor::blacklistedPaths() const
{
    Q_D(const FSMonitor);
    return d->excludeMatcher.patterns();
}

void FSMonitor::setMaxResourceUsage(double percentage)
{
    Q_D(FSMonitor);
    // Clamp between 0.1 and 1.0
    d->maxUsagePercentage = qBound(0.1, percentage, 1.0);
}

double FSMonitor::maxResourceUsage() const
{
    Q_D(const FSMonitor);
    return d->maxUsagePercentage;
}

int FSMonitor::currentWatchCount() const
{
    Q_D(const FSMonitor);
    return d->watchedDirectories.size();
}

int FSMonitor::maxAvailableWatchCount() const
{
    Q_D(const FSMonitor);
    return d->maxWatches;
}

void FSMonitor::setUseFastScan(bool enable)
{
    Q_D(FSMonitor);

    if (d->active) {
        fmWarning() << "FSMonitor: Cannot change fast scan setting while monitor is active";
        return;
    }

    d->useFastScan = enable;
}

bool FSMonitor::useFastScan() const
{
    Q_D(const FSMonitor);
    return d->useFastScan;
}

SERVICETEXTINDEX_END_NAMESPACE
