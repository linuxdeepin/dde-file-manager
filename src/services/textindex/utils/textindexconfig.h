// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TEXTINDEXCONFIG_H
#define TEXTINDEXCONFIG_H

#include "service_textindex_global.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMutex>

SERVICETEXTINDEX_BEGIN_NAMESPACE
class TextIndexConfig : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TextIndexConfig)

public:
    static TextIndexConfig &instance();

    // Getter methods for each configuration
    int autoIndexUpdateInterval() const;
    qint64 inotifyResourceCleanupDelayMs() const;
    int maxIndexFileSizeMB() const;
    QStringList supportedFileExtensions() const;
    bool indexHiddenFiles() const;
    QStringList folderExcludeFilters() const;
    int cpuUsageLimitPercent() const;
    double inotifyWatchesCoefficient() const;

    // Call this if you need to manually reload all configurations
    Q_INVOKABLE void reloadConfig();

signals:
    void configChanged();   // Emitted after configurations are reloaded

private:
    explicit TextIndexConfig(QObject *parent = nullptr);
    ~TextIndexConfig() override = default;

    void loadAllConfigs();
    void setupConnections();

    DFMBASE_NAMESPACE::DConfigManager *m_dconfigManager;

    // Member variables to store the loaded configuration values
    int m_autoIndexUpdateInterval;
    qint64 m_inotifyResourceCleanupDelayMs;
    int m_maxIndexFileSizeMB;
    QStringList m_supportedFileExtensions;
    bool m_indexHiddenFiles;
    QStringList m_folderExcludeFilters;
    int m_cpuUsageLimitPercent;
    double m_inotifyWatchesCoefficient;

    mutable QMutex m_mutex;

    // Default values (matching your JSON for robustness)
    static const int DEFAULT_AUTO_INDEX_UPDATE_INTERVAL = 180;
    static const qint64 DEFAULT_INOTIFY_RESOURCE_CLEANUP_DELAY = 1800LL;   // 30 * 60 * 1000
    static const int DEFAULT_MAX_INDEX_FILE_SIZE_MB = 50;
    static const bool DEFAULT_INDEX_HIDDEN_FILES = false;
    static const int DEFAULT_CPU_USAGE_LIMIT_PERCENT = 50;
    static constexpr double DEFAULT_INOTIFY_WATCHES_COEFFICIENT = 0.5;
    // Default QStringLists need to be initialized in the .cpp or constructor
    // For simplicity here, we'll define them directly in loadAllConfigs logic
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // TEXTINDEXCONFIG_H
