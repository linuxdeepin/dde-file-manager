// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fseventcontroller.h"

#include <utils/textindexconfig.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

FSEventController::FSEventController(QObject *parent)
    : QObject { parent }
{
}

void FSEventController::setupFSEventCollector()
{
    m_fsEventCollector = std::make_unique<FSEventCollector>(this);

    // FSEventCollector uses event collection interval (3 seconds by default)
    m_collectorIntervalSecs = TextIndexConfig::instance().autoIndexUpdateInterval();
    m_fsEventCollector->setCollectionInterval(m_collectorIntervalSecs);
    m_fsEventCollector->setMaxEventCount(10000);   // Default 10k events

    // FSEventController uses monitoring start delay (3 seconds by default)
    m_monitoringStartDelaySecs = TextIndexConfig::instance().monitoringStartDelaySeconds();

    // FSEventController uses silent start delay (180 seconds by default)
    m_silentStartDelaySecs = TextIndexConfig::instance().silentIndexUpdateDelay();

    connect(m_fsEventCollector.get(), &FSEventCollector::filesCreated,
            this, &FSEventController::onFilesCreated);
    connect(m_fsEventCollector.get(), &FSEventCollector::filesDeleted,
            this, &FSEventController::onFilesDeleted);
    connect(m_fsEventCollector.get(), &FSEventCollector::filesModified,
            this, &FSEventController::onFilesModified);
    connect(m_fsEventCollector.get(), &FSEventCollector::filesMoved,
            this, &FSEventController::onFilesMoved);
    connect(m_fsEventCollector.get(), &FSEventCollector::flushFinished,
            this, &FSEventController::onFlushFinished);

    // Connect to configuration changes to dynamically update collection interval
    connect(&TextIndexConfig::instance(), &TextIndexConfig::configChanged,
            this, &FSEventController::onConfigChanged);

    // Create separate timers for monitoring start and silent start
    m_monitoringStartTimer = new QTimer(this);
    m_silentStartTimer = new QTimer(this);
    m_stopTimer = new QTimer(this);
    m_monitoringStartTimer->setSingleShot(true);
    m_silentStartTimer->setSingleShot(true);
    m_stopTimer->setSingleShot(true);

    // Monitoring start timer - only responsible for starting monitoring
    connect(m_monitoringStartTimer, &QTimer::timeout, this, [this]() {
        if (!m_enabled) {
            fmWarning() << "Cannot start monitor, enabled state has been changed";
            return;
        }
        emit monitoring(true);
    });

    // Silent start timer - only responsible for requesting silent start
    connect(m_silentStartTimer, &QTimer::timeout, this, [this]() {
        if (!m_enabled) {
            fmWarning() << "Cannot trigger silent start, enabled state has been changed";
            return;
        }
        emit requestSlientStart();
    });

    connect(m_stopTimer, &QTimer::timeout, this, [this]() {
        if (m_enabled) {
            fmWarning() << "Cannot stop monitor, enabled state has been changed";
            return;
        }
        emit monitoring(false);
    });
}

bool FSEventController::isEnabled() const
{
    return m_enabled;
}

void FSEventController::setEnabled(bool enabled)
{
    m_enabled = enabled;

    fmInfo() << "FSEventController: Enabled state changed to:" << m_enabled;
    if (m_enabled) {
        m_stopTimer->stop();
        m_lastSilentlyFlag = silentlyRefreshStarted();

        // Start monitoring timer based on silent flag
        if (silentlyRefreshStarted()) {
            // Use monitoring start delay for first start (3 seconds by default)
            m_monitoringStartTimer->start(m_monitoringStartDelaySecs * 1000);
            // Use silent start delay for silent start (180 seconds by default)
            m_silentStartTimer->start(m_silentStartDelaySecs * 1000);
            setSilentlyRefreshStarted(false);
        } else {
            // Start monitoring immediately
            m_monitoringStartTimer->start(0);
        }
    } else {
        m_monitoringStartTimer->stop();
        m_silentStartTimer->stop();
        // 停止监控将清除所有的监控目录，重建需要极大的开销，因此延迟清理资源
        m_stopTimer->start(TextIndexConfig::instance().inotifyResourceCleanupDelayMs());
    }
}

void FSEventController::setEnabledNow(bool enabled)
{
    if (enabled) {
        setEnabled(enabled);
    } else {
        m_enabled = false;
        stopFSMonitoring();
    }
}

void FSEventController::startFSMonitoring()
{
    if (!m_fsEventCollector) {
        fmWarning() << "FSEventController: Cannot start monitoring, FSEventCollector not initialized";
        return;
    }

    if (m_fsEventCollector->isActive()) {
        fmInfo() << "FSEventController: FS monitoring already active";
        return;
    }

    // Initialize with default indexed directories
    QStringList indexedDirs = DFMSEARCH::Global::defaultIndexedDirectory();
    if (indexedDirs.isEmpty()) {
        fmWarning() << "FSEventController: No default indexed directories found";
        return;
    }

    bool success = m_fsEventCollector->initialize(indexedDirs);
    if (!success) {
        fmWarning() << "FSEventController: Failed to initialize FSEventCollector";
        return;
    }

    // Clear any previously collected events
    clearCollections();

    // Start the collector
    success = m_fsEventCollector->start();
    if (!success) {
        fmWarning() << "FSEventController: Failed to start FSEventCollector";
        return;
    }

    fmInfo() << "FSEventController: FS monitoring started successfully with" << indexedDirs.size() << "directories";
}

void FSEventController::stopFSMonitoring()
{
    if (!m_fsEventCollector || !m_fsEventCollector->isActive()) {
        return;
    }

    m_fsEventCollector->stop();

    // Clear any collected events
    clearCollections();

    fmInfo() << "FSEventController: FS monitoring stopped";
}

void FSEventController::setSilentlyRefreshStarted(bool flag)
{
    m_silentlyFlag = flag;
}

bool FSEventController::silentlyRefreshStarted() const
{
    return m_silentlyFlag;
}

void FSEventController::onFilesCreated(const QStringList &paths)
{
    if (!m_enabled) {
        return;
    }

    fmDebug() << "FSEventController: Files created event -" << paths.size() << "items";
    m_collectedCreatedFiles.append(paths);
}

void FSEventController::onFilesDeleted(const QStringList &paths)
{
    if (!m_enabled) {
        return;
    }

    fmDebug() << "FSEventController: Files deleted event -" << paths.size() << "items";
    m_collectedDeletedFiles.append(paths);
}

void FSEventController::onFilesModified(const QStringList &paths)
{
    if (!m_enabled) {
        return;
    }

    fmDebug() << "FSEventController: Files modified event -" << paths.size() << "items";
    m_collectedModifiedFiles.append(paths);
}

void FSEventController::onFilesMoved(const QHash<QString, QString> &movedPaths)
{
    if (!m_enabled) {
        return;
    }

    fmDebug() << "FSEventController: Files moved event -" << movedPaths.size() << "items";

    // Merge the moved paths into our collection
    for (auto it = movedPaths.constBegin(); it != movedPaths.constEnd(); ++it) {
        m_collectedMovedFiles.insert(it.key(), it.value());
    }
}

void FSEventController::onFlushFinished()
{
    if (!m_enabled) {
        return;
    }

    fmDebug() << "FSEventController: Flush finished, processing events";

    // Check if we have any events to process
    if (m_collectedCreatedFiles.isEmpty() && m_collectedModifiedFiles.isEmpty()
        && m_collectedDeletedFiles.isEmpty() && m_collectedMovedFiles.isEmpty()) {
        fmDebug() << "FSEventController: No file system events to process";
        return;
    }

    fmDebug() << "FSEventController: Processing file changes - Created:" << m_collectedCreatedFiles.size()
              << "Modified:" << m_collectedModifiedFiles.size()
              << "Deleted:" << m_collectedDeletedFiles.size()
              << "Moved:" << m_collectedMovedFiles.size();

    // Process file moves separately for optimization
    if (!m_collectedMovedFiles.isEmpty()) {
        emit requestProcessFileMoves(m_collectedMovedFiles);
    }

    // Process regular file changes (create, modify, delete)
    if (!m_collectedCreatedFiles.isEmpty() || !m_collectedModifiedFiles.isEmpty() || !m_collectedDeletedFiles.isEmpty()) {
        emit requestProcessFileChanges(m_collectedCreatedFiles, m_collectedModifiedFiles, m_collectedDeletedFiles);
    }

    clearCollections();
}

void FSEventController::clearCollections()
{
    m_collectedCreatedFiles.clear();
    m_collectedDeletedFiles.clear();
    m_collectedModifiedFiles.clear();
    m_collectedMovedFiles.clear();
}

void FSEventController::onConfigChanged()
{
    const int newIntervalSecs = TextIndexConfig::instance().autoIndexUpdateInterval();
    const int newMonitoringDelaySecs = TextIndexConfig::instance().monitoringStartDelaySeconds();
    const int newSilentDelaySecs = TextIndexConfig::instance().silentIndexUpdateDelay();

    // Update event collection interval for FSEventCollector
    if (newIntervalSecs != m_collectorIntervalSecs) {
        fmInfo() << "FSEventController: Collection interval changed from"
                 << m_collectorIntervalSecs << "to" << newIntervalSecs << "seconds";

        m_collectorIntervalSecs = newIntervalSecs;

        // Update the collector's interval if it exists
        if (m_fsEventCollector) {
            m_fsEventCollector->setCollectionInterval(m_collectorIntervalSecs);
            fmInfo() << "FSEventController: Updated FSEventCollector collection interval to"
                     << m_collectorIntervalSecs << "seconds";
        }
    }

    // Update monitoring start delay for FSEventController
    if (newMonitoringDelaySecs != m_monitoringStartDelaySecs) {
        fmInfo() << "FSEventController: Monitoring start delay changed from"
                 << m_monitoringStartDelaySecs << "to" << newMonitoringDelaySecs << "seconds";
        m_monitoringStartDelaySecs = newMonitoringDelaySecs;
    }

    // Update silent start delay for FSEventController
    if (newSilentDelaySecs != m_silentStartDelaySecs) {
        fmInfo() << "FSEventController: Silent start delay changed from"
                 << m_silentStartDelaySecs << "to" << newSilentDelaySecs << "seconds";
        m_silentStartDelaySecs = newSilentDelaySecs;
    }
}

SERVICETEXTINDEX_END_NAMESPACE
