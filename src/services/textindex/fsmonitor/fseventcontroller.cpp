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

    m_collectorIntervalSecs = TextIndexConfig::instance().autoIndexUpdateInterval();
    m_fsEventCollector->setCollectionInterval(m_collectorIntervalSecs);
    m_fsEventCollector->setMaxEventCount(10000);   // Default 10k events

    connect(m_fsEventCollector.get(), &FSEventCollector::filesCreated,
            this, &FSEventController::onFilesCreated);
    connect(m_fsEventCollector.get(), &FSEventCollector::filesDeleted,
            this, &FSEventController::onFilesDeleted);
    connect(m_fsEventCollector.get(), &FSEventCollector::filesModified,
            this, &FSEventController::onFilesModified);
    connect(m_fsEventCollector.get(), &FSEventCollector::flushFinished,
            this, &FSEventController::onFlushFinished);

    m_startTimer = new QTimer(this);
    m_stopTimer = new QTimer(this);
    m_startTimer->setSingleShot(true);
    m_stopTimer->setSingleShot(true);

    connect(m_startTimer, &QTimer::timeout, this, [this]() {
        if (!m_enabled) {
            fmWarning() << "Cannot start monitor, enabled state has been changed";
            return;
        }
        emit monitoring(true);

        if (m_lastSilentlyFlag)
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

    fmInfo() << "FSEventController enabled: " << m_enabled;
    if (m_enabled) {
        m_stopTimer->stop();
        m_lastSilentlyFlag = silentlyRefreshStarted();
        if (silentlyRefreshStarted()) {
            m_startTimer->start(m_collectorIntervalSecs * 1000);
            setSilentlyRefreshStarted(false);
        } else {
            m_startTimer->start(0);
        }
    } else {
        m_startTimer->stop();
        // 停止监控将清除所有的监控目录，重建需要极大的开销，因此延迟清理资源
        m_stopTimer->start(TextIndexConfig::instance().inotifyResourceCleanupDelayMs());
    }
}

void FSEventController::setEnabledNow(bool enabled)
{
    if (enabled)
        setEnabled(enabled);
    else
        stopFSMonitoring();
}

void FSEventController::startFSMonitoring()
{
    if (!m_fsEventCollector) {
        return;
    }

    if (m_fsEventCollector->isActive()) {
        fmInfo() << "FS monitoring already active";
        return;
    }

    // Initialize with default indexed directories
    QStringList indexedDirs = DFMSEARCH::Global::defaultIndexedDirectory();
    if (indexedDirs.isEmpty()) {
        fmWarning() << "No default indexed directories found";
        return;
    }

    bool success = m_fsEventCollector->initialize(indexedDirs);
    if (!success) {
        fmWarning() << "Failed to initialize FSEventCollector";
        return;
    }

    // Clear any previously collected events
    clearCollections();

    // Start the collector
    success = m_fsEventCollector->start();
    if (!success) {
        fmWarning() << "Failed to start FSEventCollector";
        return;
    }

    fmInfo() << "FS monitoring started successfully with" << indexedDirs.size() << "directories";
}

void FSEventController::stopFSMonitoring()
{
    if (!m_fsEventCollector || !m_fsEventCollector->isActive()) {
        return;
    }

    m_fsEventCollector->stop();

    // Clear any collected events
    clearCollections();

    fmInfo() << "FS monitoring stopped";
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

    fmInfo() << "FS event: Files created -" << paths.size() << "items";
    m_collectedCreatedFiles.append(paths);
}

void FSEventController::onFilesDeleted(const QStringList &paths)
{
    if (!m_enabled) {
        return;
    }

    fmInfo() << "FS event: Files deleted -" << paths.size() << "items";
    m_collectedDeletedFiles.append(paths);
}

void FSEventController::onFilesModified(const QStringList &paths)
{
    if (!m_enabled) {
        return;
    }

    fmInfo() << "FS event: Files modified -" << paths.size() << "items";
    m_collectedModifiedFiles.append(paths);
}

void FSEventController::onFlushFinished()
{
    if (!m_enabled) {
        return;
    }

    fmInfo() << "FS event: Flush finished, processing events";

    // Check if we have any events to process
    if (m_collectedCreatedFiles.isEmpty() && m_collectedModifiedFiles.isEmpty() && m_collectedDeletedFiles.isEmpty()) {
        fmInfo() << "No file system events to process";
        return;
    }

    fmInfo() << "Processing file changes - Created:" << m_collectedCreatedFiles.size()
             << "Modified:" << m_collectedModifiedFiles.size()
             << "Deleted:" << m_collectedDeletedFiles.size();

    emit requestProcessFileChanges(m_collectedCreatedFiles, m_collectedModifiedFiles, m_collectedDeletedFiles);

    clearCollections();
}

void FSEventController::clearCollections()
{
    m_collectedCreatedFiles.clear();
    m_collectedDeletedFiles.clear();
    m_collectedModifiedFiles.clear();
}

SERVICETEXTINDEX_END_NAMESPACE
