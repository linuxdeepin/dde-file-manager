// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "textindexconfig.h"

#include <QDebug>

SERVICETEXTINDEX_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

TextIndexConfig &TextIndexConfig::instance()
{
    static TextIndexConfig self;
    return self;
}

TextIndexConfig::TextIndexConfig(QObject *parent)
    : QObject(parent),
      m_dconfigManager(DConfigManager::instance())   // Get DConfigManager instance
{
    QString err;
    if (!DFMBASE_NAMESPACE::DConfigManager::instance()->addConfig(Defines::DConf::kTextIndexSchema, &err))
        fmWarning() << "TextIndexConfig: Failed to load DConfig schema:" << err;

    loadAllConfigs();
    setupConnections();
}

void TextIndexConfig::setupConnections()
{
    // Connect to DConfigManager's signal for changes in our schema
    // This allows automatic reloading if the config is changed externally (e.g., by dconf-editor)
    connect(m_dconfigManager, &DConfigManager::valueChanged, this,
            [this](const QString &schema, const QString &key) {
                if (schema == Defines::DConf::kTextIndexSchema) {
                    fmDebug() << "TextIndexConfig: DConfig changed for schema:" << schema << "key:" << key;
                    // You could be more granular and only reload the specific key,
                    // but reloading all is simpler and often acceptable.
                    loadAllConfigs();
                    emit configChanged();
                }
            });
}

void TextIndexConfig::loadAllConfigs()
{
    QMutexLocker locker(&m_mutex);
    fmDebug() << "TextIndexConfig: Loading text index configurations";

    // Auto Index Update Interval (FSEventCollector event collection interval)
    m_autoIndexUpdateInterval = m_dconfigManager->value(
                                                        Defines::DConf::kTextIndexSchema,
                                                        Defines::DConf::kAutoIndexUpdateInterval,
                                                        DEFAULT_AUTO_INDEX_UPDATE_INTERVAL)
                                        .toInt();
    // Validate autoIndexUpdateInterval
    if (m_autoIndexUpdateInterval < 1 || m_autoIndexUpdateInterval > 3600) {
        fmWarning() << "TextIndexConfig: Invalid autoIndexUpdateInterval value:" << m_autoIndexUpdateInterval
                    << ", using default:" << DEFAULT_AUTO_INDEX_UPDATE_INTERVAL;
        m_autoIndexUpdateInterval = DEFAULT_AUTO_INDEX_UPDATE_INTERVAL;
    }

    // Monitoring Start Delay (FSEventController monitoring start delay)
    m_monitoringStartDelaySeconds = m_dconfigManager->value(
                                                            Defines::DConf::kTextIndexSchema,
                                                            Defines::DConf::kMonitoringStartDelaySeconds,
                                                            DEFAULT_MONITORING_START_DELAY_SECONDS)
                                            .toInt();
    // Validate monitoringStartDelaySeconds
    if (m_monitoringStartDelaySeconds < 0 || m_monitoringStartDelaySeconds > 3600) {
        fmWarning() << "TextIndexConfig: Invalid monitoringStartDelaySeconds value:" << m_monitoringStartDelaySeconds
                    << ", using default:" << DEFAULT_MONITORING_START_DELAY_SECONDS;
        m_monitoringStartDelaySeconds = DEFAULT_MONITORING_START_DELAY_SECONDS;
    }

    // Silent Index Update Delay (FSEventController first start delay)
    m_silentIndexUpdateDelay = m_dconfigManager->value(
                                                       Defines::DConf::kTextIndexSchema,
                                                       Defines::DConf::kSilentIndexUpdateDelay,
                                                       DEFAULT_SILENT_INDEX_UPDATE_DELAY)
                                       .toInt();
    // Validate silentIndexUpdateDelay
    if (m_silentIndexUpdateDelay < 1 || m_silentIndexUpdateDelay > 3600) {
        fmWarning() << "TextIndexConfig: Invalid silentIndexUpdateDelay value:" << m_silentIndexUpdateDelay
                    << ", using default:" << DEFAULT_SILENT_INDEX_UPDATE_DELAY;
        m_silentIndexUpdateDelay = DEFAULT_SILENT_INDEX_UPDATE_DELAY;
    }

    // Inotify Resource Cleanup Delay
    m_inotifyResourceCleanupDelayMs = m_dconfigManager->value(
                                                              Defines::DConf::kTextIndexSchema,
                                                              Defines::DConf::kInotifyResourceCleanupDelay,   // Ensure this matches JSON and Defines.h
                                                              DEFAULT_INOTIFY_RESOURCE_CLEANUP_DELAY)
                                              .toLongLong();

    // Max Index File Size MB
    m_maxIndexFileSizeMB = m_dconfigManager->value(
                                                   Defines::DConf::kTextIndexSchema,
                                                   Defines::DConf::kMaxIndexFileSizeMB,
                                                   DEFAULT_MAX_INDEX_FILE_SIZE_MB)
                                   .toInt();

    // Max Index File Truncation Size MB
    m_maxIndexFileTruncationSizeMB = m_dconfigManager->value(
                                                             Defines::DConf::kTextIndexSchema,
                                                             Defines::DConf::kMaxIndexFileTruncationSizeMB,
                                                             DEFAULT_MAX_INDEX_FILE_TRUNCATION_SIZE_MB)
                                             .toInt();
    // Validate and apply default if value is invalid (negative, zero, or too large)
    if (m_maxIndexFileTruncationSizeMB <= 0 || m_maxIndexFileTruncationSizeMB > 1024) {
        fmWarning() << "TextIndexConfig: Invalid maxIndexFileTruncationSizeMB value:" << m_maxIndexFileTruncationSizeMB << ", using default:" << DEFAULT_MAX_INDEX_FILE_TRUNCATION_SIZE_MB;
        m_maxIndexFileTruncationSizeMB = DEFAULT_MAX_INDEX_FILE_TRUNCATION_SIZE_MB;
    }

    // Supported File Extensions
    const QStringList defaultSupportedExtensions = {
        "rtf", "odt", "ods", "odp", "odg", "docx",
        "xlsx", "pptx", "ppsx", "md", "xls", "xlsb",
        "doc", "dot", "wps", "ppt", "pps", "txt",
        "pdf", "dps", "sh", "html", "htm", "xml",
        "xhtml", "dhtml", "shtm", "shtml", "json",
        "css", "yaml", "ini", "bat", "js", "sql",
        "uof", "ofd"
    };
    m_supportedFileExtensions = m_dconfigManager->value(
                                                        Defines::DConf::kTextIndexSchema,
                                                        Defines::DConf::kSupportedFileExtensions,
                                                        QVariant::fromValue(defaultSupportedExtensions))   // Pass QVariant holding QStringList
                                        .toStringList();

    // Index Hidden Files
    m_indexHiddenFiles = m_dconfigManager->value(
                                                 Defines::DConf::kTextIndexSchema,
                                                 Defines::DConf::kIndexHiddenFiles,
                                                 DEFAULT_INDEX_HIDDEN_FILES)
                                 .toBool();

    // Folder Exclude Filters
    const QStringList defaultFolderExcludeFilters = {
        ".git", ".svn", ".hg", ".cache", ".local/share/Trash", ".Trash",
        ".thumbnails", "thumbnails", ".mozilla", "CMakeFiles",
        "CMakeTmp", "CMakeTmpQmake", "lost+found"
    };
    m_folderExcludeFilters = m_dconfigManager->value(
                                                     Defines::DConf::kTextIndexSchema,
                                                     Defines::DConf::kFolderExcludeFilters,
                                                     QVariant::fromValue(defaultFolderExcludeFilters))   // Pass QVariant holding QStringList
                                     .toStringList();

    // CPU isage limit percent
    m_cpuUsageLimitPercent = m_dconfigManager->value(
                                                     Defines::DConf::kTextIndexSchema,
                                                     Defines::DConf::kCpuUsageLimitPercent,
                                                     DEFAULT_CPU_USAGE_LIMIT_PERCENT)
                                     .toInt();
    if (m_cpuUsageLimitPercent < 10 || m_cpuUsageLimitPercent >= 100) {
        m_cpuUsageLimitPercent = DEFAULT_CPU_USAGE_LIMIT_PERCENT;
    }

    // Inotify watches coefficient
    m_inotifyWatchesCoefficient = m_dconfigManager->value(
                                                          Defines::DConf::kTextIndexSchema,
                                                          Defines::DConf::kInotifyWatchesCoefficient,
                                                          DEFAULT_INOTIFY_WATCHES_COEFFICIENT)
                                          .toDouble();
    if (m_inotifyWatchesCoefficient < 0.1 || m_inotifyWatchesCoefficient > 1.0) {
        m_inotifyWatchesCoefficient = DEFAULT_INOTIFY_WATCHES_COEFFICIENT;
    }

    // Batch commit interval
    m_batchCommitInterval = m_dconfigManager->value(
                                                    Defines::DConf::kTextIndexSchema,
                                                    Defines::DConf::kBatchCommitInterval,
                                                    DEFAULT_BATCH_COMMIT_INTERVAL)
                                    .toInt();
    if (m_batchCommitInterval < 100 || m_batchCommitInterval > 10000) {
        m_batchCommitInterval = DEFAULT_BATCH_COMMIT_INTERVAL;
    }

    fmDebug() << "TextIndexConfig: Text index configurations loaded successfully";
    // You might want to print the loaded values here for debugging if needed
    // fmDebug() << "AutoIndexUpdateInterval:" << m_autoIndexUpdateInterval;
    // ... and so on
}

void TextIndexConfig::reloadConfig()
{
    loadAllConfigs();
    emit configChanged();
}

// --- Getter Implementations ---
int TextIndexConfig::autoIndexUpdateInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_autoIndexUpdateInterval;
}

int TextIndexConfig::monitoringStartDelaySeconds() const
{
    QMutexLocker locker(&m_mutex);
    return m_monitoringStartDelaySeconds;
}

int TextIndexConfig::silentIndexUpdateDelay() const
{
    QMutexLocker locker(&m_mutex);
    return m_silentIndexUpdateDelay;
}

qint64 TextIndexConfig::inotifyResourceCleanupDelayMs() const
{
    QMutexLocker locker(&m_mutex);
    return m_inotifyResourceCleanupDelayMs;
}

int TextIndexConfig::maxIndexFileSizeMB() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxIndexFileSizeMB;
}

int TextIndexConfig::maxIndexFileTruncationSizeMB() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxIndexFileTruncationSizeMB;
}

QStringList TextIndexConfig::supportedFileExtensions() const
{
    QMutexLocker locker(&m_mutex);
    return m_supportedFileExtensions;
}

bool TextIndexConfig::indexHiddenFiles() const
{
    QMutexLocker locker(&m_mutex);
    return m_indexHiddenFiles;
}

QStringList TextIndexConfig::folderExcludeFilters() const
{
    QMutexLocker locker(&m_mutex);
    return m_folderExcludeFilters;
}

int TextIndexConfig::cpuUsageLimitPercent() const
{
    QMutexLocker locker(&m_mutex);
    return m_cpuUsageLimitPercent;
}

double TextIndexConfig::inotifyWatchesCoefficient() const
{
    QMutexLocker locker(&m_mutex);
    return m_inotifyWatchesCoefficient;
}

int TextIndexConfig::batchCommitInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_batchCommitInterval;
}

SERVICETEXTINDEX_END_NAMESPACE
