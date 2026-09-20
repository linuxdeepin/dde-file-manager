// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fseventcontroller.h"

#include "utils/pathexcludematcher.h"
#include "utils/textindexconfig.h"

#include <dfm-search/dsearch_global.h>

#include <QFileInfo>
#include <memory>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

// Debounce window for lost-event recovery so a storm of disconnects or
// overflow reports coalesces into a single compensating update task.
constexpr int kEventsRecoveryDebounceMs = 5000;

}

FSEventController::FSEventController(IndexProfile profile, QObject *parent)
    : QObject { parent },
      m_profile(std::move(profile))
{
}

void FSEventController::setupFSEventCollector()
{
    // Per-runtime blacklist matcher:
    // - profile 提供黑名单（filename: anything blacklist + 索引目录 + .avfs，死循环防护）
    // - 未提供时沿用全局匹配器（Content/Ocr: TextIndexConfig folderExcludeFilters
    //   + anything blacklist，保持现有行为不变）
    std::shared_ptr<PathExcludeMatcher> filterMatcher;
    const auto &filterPolicy = m_profile.filterPolicy();
    if (filterPolicy.blacklistProvider) {
        filterMatcher = std::make_shared<PathExcludeMatcher>(filterPolicy.blacklistProvider());
    } else {
        filterMatcher = std::make_shared<PathExcludeMatcher>(PathExcludeMatcher::createForIndex());
    }

    m_fsEventCollector = std::make_unique<FSEventCollector>(
            [this, filterMatcher](const QString &path) {
                // Per-profile symlink policy: link 条目仅 filename profile 收录
                // （FSMonitor 事件分发已放行 symlink，这里按策略拦截，Content/Ocr
                // 行为保持不变——symlink 事件照旧被丢弃）
                if (!m_profile.filterPolicy().indexSymlinks && QFileInfo(path).isSymLink())
                    return false;

                if (!m_profile.isCandidateFile(path))
                    return false;

                // Per-profile hidden file filtering：三类入口（全盘遍历/增量路径列表/
                // 事件谓词）共用 IndexProfile::shouldSkipHiddenEntry，保证判定一致——
                // Content/Ocr 丢弃所有隐藏条目事件；filename 丢弃屏蔽策略命中的
                // 条目事件（默认主目录下的隐藏条目）
                if (m_profile.shouldSkipHiddenEntry(QFileInfo(path).absoluteFilePath()))
                    return false;

                // Per-profile blacklist filtering
                if (filterMatcher->shouldExclude(path))
                    return false;

                return true;
            },
            this);

    // Use runtime policy interval if specified, otherwise fall back to config
    const auto &rp = m_profile.runtimePolicy();
    if (rp.eventCollectionWindowMs > 0) {
        m_fsEventCollector->setCollectionIntervalMs(rp.eventCollectionWindowMs);
    } else {
        m_collectorIntervalSecs = TextIndexConfig::instance().autoIndexUpdateInterval();
        m_fsEventCollector->setCollectionInterval(m_collectorIntervalSecs);
    }
    m_fsEventCollector->setMaxEventCount(10000);   // Default 10k events

    // Silent start delay: profile-specific recovery delay takes precedence
    // (filename: 30s to shorten the updateInProgress degraded-search window),
    // otherwise fall back to the global config (Content/Ocr, default 180s).
    if (rp.recoveryUpdateDelayMs > 0) {
        m_silentStartDelayMs = rp.recoveryUpdateDelayMs;
    } else {
        m_silentStartDelayMs = TextIndexConfig::instance().silentIndexUpdateDelay() * 1000;
    }

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

    // Lost-event recovery: debounce repeated eventsLost notifications, then
    // ask the runtimes to schedule a compensating full update task.
    m_recoveryTimer = new QTimer(this);
    m_recoveryTimer->setSingleShot(true);
    m_recoveryTimer->setInterval(kEventsRecoveryDebounceMs);
    connect(m_recoveryTimer, &QTimer::timeout, this, [this]() {
        if (!m_enabled) {
            return;
        }

        fmInfo() << "FSEventController: requesting events recovery update";
        emit requestEventsRecovery();
    });

    connect(&FSMonitor::instance(), &FSMonitor::eventsLost,
            this, &FSEventController::onEventsLost);

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
        emit requestSilentStart();
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

        // Always start monitoring immediately — no artificial delay.
        m_monitoringStartTimer->start(0);

        // On first start (silentlyRefreshStarted), schedule a delayed
        // silent index update to avoid heavy I/O during system boot.
        if (silentlyRefreshStarted()) {
            m_silentStartTimer->start(m_silentStartDelayMs);
            setSilentlyRefreshStarted(false);
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

void FSEventController::onEventsLost()
{
    if (!m_enabled) {
        return;
    }

    fmWarning() << "FSEventController: filesystem events lost, scheduling recovery update";
    m_recoveryTimer->start();
}

void FSEventController::onConfigChanged()
{
    const int newIntervalSecs = TextIndexConfig::instance().autoIndexUpdateInterval();
    const int newSilentDelaySecs = TextIndexConfig::instance().silentIndexUpdateDelay();

    // Update event collection interval for FSEventCollector
    // Skip if profile uses a custom interval (runtime policy)
    if (m_profile.runtimePolicy().eventCollectionWindowMs <= 0 && newIntervalSecs != m_collectorIntervalSecs) {
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

    // Update silent start delay for FSEventController
    // Skip if profile uses a custom recovery delay (runtime policy)
    if (m_profile.runtimePolicy().recoveryUpdateDelayMs <= 0
        && newSilentDelaySecs * 1000 != m_silentStartDelayMs) {
        fmInfo() << "FSEventController: Silent start delay changed from"
                 << m_silentStartDelayMs << "to" << newSilentDelaySecs * 1000 << "ms";
        m_silentStartDelayMs = newSilentDelaySecs * 1000;
    }
}

SERVICETEXTINDEX_END_NAMESPACE
