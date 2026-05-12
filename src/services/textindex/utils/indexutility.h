// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef INDEXUTILITY_H
#define INDEXUTILITY_H

#include "service_textindex_global.h"

#include <QFileInfo>
#include <QMutex>

#include <dconfig.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexStateStore;

namespace IndexUtility {

/**
 * @brief Index state for crash recovery
 */
enum class IndexState {
    Clean,   ///< Index is complete, last shutdown was clean with no pending tasks
    Dirty,   ///< Index may be incomplete, needs global update on next start
    Unknown   ///< State field not found (legacy status file or corrupted)
};

bool isIndexWithAnything(const QString &path);
bool isDefaultIndexedDirectory(const QString &path);

/**
 * @brief Check if a file size is within the allowed limit for indexing
 * @param fileInfo QFileInfo object of the file to check
 * @return true if file size is acceptable, false otherwise
 */
bool checkFileSize(const QFileInfo &fileInfo, qint64 sizeMBFromConfig);

/**
 * @brief Check if a content file is supported for indexing
 * @param path File path to check
 * @return true if file type is supported and meets size requirements, false otherwise
 */
bool isSupportedTextFile(const QString &path);

/**
 * @brief Check if a OCR file is supported for indexing
 * @param path File path to check
 * @return true if file type is supported and meets size requirements, false otherwise
 */
bool isSupportedOCRFile(const QString &path);

class AnythingConfigWatcher : public QObject
{
    Q_OBJECT
public:
    static AnythingConfigWatcher *instance();
    ~AnythingConfigWatcher() override;

    QStringList defaultAnythingIndexPaths();
    QStringList defaultAnythingIndexPathsRealtime();
    QStringList defaultBlacklistPaths();
    QStringList defaultBlacklistPathsRealtime();

Q_SIGNALS:
    void rebuildRequired(const QString &reason);

private slots:
    void handleConfigChanged(const QString &key);

private:
    explicit AnythingConfigWatcher(QObject *parent = nullptr);

private:
    DTK_CORE_NAMESPACE::DConfig *cfg { nullptr };
    QMutex mu;
    QStringList defaultIndexPath;
    QStringList blacklistPaths;
};

/**
 * @brief Generic DConfig watcher that emits rebuildRequired when watched keys change
 *
 * Provides a reusable mechanism to monitor multiple DConfig sources
 * and trigger index rebuilds when configuration changes are detected.
 */
class ConfigRebuildWatcher : public QObject
{
    Q_OBJECT
public:
    struct WatchEntry {
        QString appId;
        QString configId;
        QString key;
    };

    explicit ConfigRebuildWatcher(const QList<WatchEntry> &entries, QObject *parent = nullptr);
    ~ConfigRebuildWatcher() override;

Q_SIGNALS:
    void rebuildRequired(const QString &reason);

private:
    struct ConfigHolder {
        DTK_CORE_NAMESPACE::DConfig *config { nullptr };
        QString appId;
        QString configId;
        QString watchedKey;
    };
    QList<ConfigHolder> m_configs;
};

/**
 * @brief Watches dlnfs/ulnfs configuration changes that affect file indexing
 *
 * Monitors two DConfig sources:
 * - org.deepin.dde.file-manager: dfm.mount.dlnfs
 * - org.deepin.dlnfs: ulnfs
 *
 * When any watched key changes, emits rebuildRequired to trigger index rebuild.
 */
class DlnfsConfigWatcher : public QObject
{
    Q_OBJECT
public:
    static DlnfsConfigWatcher *instance();
    ~DlnfsConfigWatcher() override;

Q_SIGNALS:
    void rebuildRequired(const QString &reason);

private:
    explicit DlnfsConfigWatcher(QObject *parent = nullptr);
    ConfigRebuildWatcher *m_watcher { nullptr };
};

}   // namespace IndexUtility

namespace PathCalculator {

/**
 * @brief Calculate new path for directory move operation
 * @param oldPath Original file path
 * @param fromDirPath Source directory path (normalized with trailing slash)
 * @param toDirPath Target directory path
 * @return New calculated path
 */
QString calculateNewPathForDirectoryMove(const QString &oldPath,
                                         const QString &fromDirPath,
                                         const QString &toDirPath);

/**
 * @brief Normalize directory path by ensuring it ends with '/'
 * @param dirPath Directory path to normalize
 * @return Normalized directory path
 */
QString normalizeDirectoryPath(const QString &dirPath);

/**
 * @brief Detect if the move operation is a directory move
 * @param toPath Target path to check
 * @return true if it's a directory move, false for file move
 */
bool isDirectoryMove(const QString &toPath);

/**
 * @brief Extract all ancestor directories from a file path
 * @param filePath Full file path
 * @return List of all ancestor directories in order from root to parent
 */
QStringList extractAncestorPaths(const QString &filePath);

}   // namespace PathCalculator

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXUTILITY_H
