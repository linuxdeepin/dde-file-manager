// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef INDEXUTILITY_H
#define INDEXUTILITY_H

#include "service_textindex_global.h"

#include <QFileInfo>
#include <QMutex>

#include <dconfig.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace IndexUtility {

bool isIndexWithAnything(const QString &path);
bool isDefaultIndexedDirectory(const QString &path);
bool isPathInContentIndexDirectory(const QString &path);
QString statusFilePath();
QString getLastUpdateTime();
int getIndexVersion();
bool isCompatibleVersion();

void removeIndexStatusFile();
void clearIndexDirectory();
void saveIndexStatus(const QDateTime &lastUpdateTime);
void saveIndexStatus(const QDateTime &lastUpdateTime, int version);

/**
 * @brief Check if a file size is within the allowed limit for indexing
 * @param fileInfo QFileInfo object of the file to check
 * @return true if file size is acceptable, false otherwise
 */
bool checkFileSize(const QFileInfo &fileInfo);

/**
 * @brief Check if a file is supported for indexing
 * @param path File path to check
 * @return true if file type is supported and meets size requirements, false otherwise
 */
bool isSupportedFile(const QString &path);

class AnythingConfigWatcher : public QObject
{
    Q_OBJECT
public:
    static AnythingConfigWatcher *instance();
    ~AnythingConfigWatcher() override;

    QStringList defaultAnythingIndexPaths();
    QStringList defaultAnythingIndexPathsRealtime();

private slots:
    void handleConfigChanged(const QString &key);

private:
    explicit AnythingConfigWatcher(QObject *parent = nullptr);

private:
    DTK_CORE_NAMESPACE::DConfig *cfg { nullptr };
    QMutex mu;
    QStringList defaultIndexPath;
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
