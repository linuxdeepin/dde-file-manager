// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
    if (!DFMBASE_NAMESPACE::DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.textindex", &err))
        fmWarning() << "load dconfiog failed: " << err;

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
                    qDebug() << "DConfig changed for schema:" << schema << "key:" << key;
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
    qDebug() << "Loading text index configurations...";

    // Auto Index Update Interval
    m_autoIndexUpdateInterval = m_dconfigManager->value(
                                                        Defines::DConf::kTextIndexSchema,
                                                        Defines::DConf::kAutoIndexUpdateInterval,
                                                        DEFAULT_AUTO_INDEX_UPDATE_INTERVAL)
                                        .toInt();

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

    qDebug() << "Text index configurations loaded.";
    // You might want to print the loaded values here for debugging if needed
    // qDebug() << "AutoIndexUpdateInterval:" << m_autoIndexUpdateInterval;
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

SERVICETEXTINDEX_END_NAMESPACE
