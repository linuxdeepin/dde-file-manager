// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileprovider.h"
#include "utils/indextraverseutils.h"
#include "utils/indexutility.h"
#include "utils/scopeguard.h"
#include "utils/textindexconfig.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QQueue>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

SERVICETEXTINDEX_USE_NAMESPACE

FileSystemProvider::FileSystemProvider(const QString &rootPath)
    : m_rootPath(rootPath)
{
    fmInfo() << "[FileSystemProvider] Initialized with root path:" << rootPath;
}

void FileSystemProvider::traverse(TaskState &state, const FileHandler &handler)
{
    fmInfo() << "[FileSystemProvider::traverse] Starting file system traversal from:" << m_rootPath;

    QMap<QString, QString> bindPathTable = IndexTraverseUtils::fstabBindInfo();
    QSet<QString> visitedDirs;
    QQueue<QString> dirQueue;
    dirQueue.enqueue(m_rootPath);

    int processedDirs = 0;
    int processedFiles = 0;

    while (!dirQueue.isEmpty()) {
        if (!state.isRunning()) {
            fmInfo() << "[FileSystemProvider::traverse] Traversal interrupted by user request";
            break;
        }

        QString currentPath = dirQueue.dequeue();

        // 检查是否应该跳过此目录
        if (IndexTraverseUtils::shouldSkipDirectory(currentPath)) {
            fmDebug() << "[FileSystemProvider::traverse] Skipping directory:" << currentPath;
            continue;
        }

        // 检查是否是系统目录或绑定目录
        if (!IndexUtility::isDefaultIndexedDirectory(currentPath)) {
            if (bindPathTable.contains(currentPath)) {
                fmDebug() << "[FileSystemProvider::traverse] Skipping system/bind directory:" << currentPath;
                continue;
            }
        }

        // 检查路径长度和深度限制
        if (currentPath.size() > FILENAME_MAX - 1 || currentPath.count('/') > 30) {
            fmWarning() << "[FileSystemProvider::traverse] Path too long or deep, skipping:" << currentPath
                        << "length:" << currentPath.size() << "depth:" << currentPath.count('/');
            continue;
        }

        // 检查目录是否已访问
        if (!IndexTraverseUtils::isValidDirectory(currentPath, visitedDirs)) {
            fmDebug() << "[FileSystemProvider::traverse] Directory already visited or invalid:" << currentPath;
            continue;
        }

        DIR *dir = opendir(currentPath.toStdString().c_str());
        if (!dir) {
            fmWarning() << "[FileSystemProvider::traverse] Failed to open directory:" << currentPath
                        << "error:" << strerror(errno);
            continue;
        }

        ScopeGuard dirCloser([dir]() { closedir(dir); });
        processedDirs++;

        struct dirent *entry;
        while ((entry = readdir(dir))) {
            if (!state.isRunning()) {
                fmInfo() << "[FileSystemProvider::traverse] Traversal interrupted during directory scan";
                break;
            }

            if (IndexTraverseUtils::isHiddenFile(entry->d_name) || IndexTraverseUtils::isSpecialDir(entry->d_name))
                continue;

            QString fullPath = QDir::cleanPath(currentPath + QDir::separator() + QString::fromUtf8(entry->d_name));

            struct stat st;
            if (lstat(fullPath.toStdString().c_str(), &st) == -1) {
                fmDebug() << "[FileSystemProvider::traverse] Failed to stat file:" << fullPath
                          << "error:" << strerror(errno);
                continue;
            }

            // 对于普通文件，检查路径有效性和文件扩展名
            if (S_ISREG(st.st_mode)) {
                QString fileName = QString::fromUtf8(entry->d_name);
                // 早期扩展名过滤 - 避免昂贵的路径验证
                if (!IndexTraverseUtils::isSupportedFileExtension(fileName)) {
                    continue;
                }

                // 只有通过扩展名检查的文件才进行昂贵的路径验证
                if (IndexTraverseUtils::isValidFile(fullPath)) {
                    handler(fullPath);
                    processedFiles++;
                }
            }
            // 对于目录，检查是否应该跳过，然后加入队列
            else if (S_ISDIR(st.st_mode)) {
                if (!IndexTraverseUtils::shouldSkipDirectory(fullPath)) {
                    dirQueue.enqueue(fullPath);
                }
            }
        }
    }

    fmInfo() << "[FileSystemProvider::traverse] Traversal completed - processed directories:" << processedDirs
             << "files:" << processedFiles;
}

DirectFileListProvider::DirectFileListProvider(const dfmsearch::SearchResultList &files)
    : m_fileList(files)
{
    fmInfo() << "[DirectFileListProvider] Initialized with" << files.size() << "files from search results";
}

void DirectFileListProvider::traverse(TaskState &state, const FileHandler &handler)
{
    fmInfo() << "[DirectFileListProvider::traverse] Processing" << m_fileList.size() << "files from direct list";

    int processedCount = 0;
    for (const auto &file : std::as_const(m_fileList)) {
        if (!state.isRunning()) {
            fmInfo() << "[DirectFileListProvider::traverse] Processing interrupted after" << processedCount << "files";
            break;
        }
        handler(file.path());
        processedCount++;
    }

    fmInfo() << "[DirectFileListProvider::traverse] Completed processing" << processedCount << "files";
}

qint64 DirectFileListProvider::totalCount()
{
    return m_fileList.count();
}

MixedPathListProvider::MixedPathListProvider(const QStringList &pathList)
    : m_pathList(pathList)
{
    fmInfo() << "[MixedPathListProvider] Initialized with" << pathList.size() << "paths";
}

void MixedPathListProvider::traverse(TaskState &state, const FileHandler &handler)
{
    fmInfo() << "[MixedPathListProvider::traverse] Starting traversal of" << m_pathList.size() << "mixed paths";

    // Default blacklisted directories
    QStringList defaultBlacklistedDirs = TextIndexConfig::instance().folderExcludeFilters();
    fmDebug() << "[MixedPathListProvider::traverse] Using blacklisted directories:" << defaultBlacklistedDirs;

    QSet<QString> processedFiles;   // 避免重复处理文件
    QSet<QString> visitedDirs;   // 避免目录循环引用

    // 首先处理列表中的文件和准备目录遍历
    QQueue<QString> dirQueue;
    int initialFiles = 0;
    int initialDirs = 0;

    for (const auto &path : std::as_const(m_pathList)) {
        if (!state.isRunning()) {
            fmInfo() << "[MixedPathListProvider::traverse] Initial processing interrupted";
            break;
        }

        QFileInfo fileInfo(path);
        if (!fileInfo.exists()) {
            fmWarning() << "[MixedPathListProvider::traverse] Path does not exist:" << path;
            continue;
        }

        if (fileInfo.isFile()) {
            // 早期扩展名过滤 - 避免昂贵的路径验证
            if (!IndexTraverseUtils::isSupportedFileExtension(fileInfo.fileName())) {
                fmDebug() << "[MixedPathListProvider::traverse] Skipping file with unsupported extension:" << path;
                continue;
            }

            // 处理文件 - 只有通过扩展名检查的文件才进行昂贵的路径验证
            if (IndexTraverseUtils::isValidFile(path)
                && IndexUtility::isPathInContentIndexDirectory(path)) {
                // 检查是否已经处理过这个文件
                if (!processedFiles.contains(path)) {
                    handler(path);
                    processedFiles.insert(path);
                    initialFiles++;
                } else {
                    fmDebug() << "[MixedPathListProvider::traverse] Skipping duplicate file:" << path;
                }
            } else {
                fmDebug() << "[MixedPathListProvider::traverse] Skipping invalid or out-of-scope file:" << path;
            }
        } else if (fileInfo.isDir() && !fileInfo.isSymLink()) {
            // 检查目录是否在黑名单中
            bool isBlacklisted = false;
            QString dirName = fileInfo.fileName();
            if (defaultBlacklistedDirs.contains(dirName)) {
                isBlacklisted = true;
                fmDebug() << "[MixedPathListProvider::traverse] Directory blacklisted:" << path;
            }

            // 只有不在黑名单中的目录才加入队列
            if (!isBlacklisted) {
                dirQueue.enqueue(path);
                initialDirs++;
            }
        }
    }

    fmInfo() << "[MixedPathListProvider::traverse] Initial processing completed - files:" << initialFiles
             << "directories queued:" << initialDirs;

    // 处理所有目录
    QMap<QString, QString> bindPathTable = IndexTraverseUtils::fstabBindInfo();
    int processedDirs = 0;
    int additionalFiles = 0;
    int skippedFilesByExtension = 0;   // 统计因扩展名过滤跳过的文件数

    while (!dirQueue.isEmpty()) {
        if (!state.isRunning()) {
            fmInfo() << "[MixedPathListProvider::traverse] Directory traversal interrupted";
            break;
        }

        QString currentDir = dirQueue.dequeue();

        // 检查是否是系统目录或绑定目录
        if (bindPathTable.contains(currentDir) || IndexTraverseUtils::shouldSkipDirectory(currentDir)) {
            fmDebug() << "[MixedPathListProvider::traverse] Skipping system/bind directory:" << currentDir;
            continue;
        }

        // 检查路径长度和深度限制
        if (currentDir.size() > FILENAME_MAX - 1 || currentDir.count('/') > 30) {
            fmWarning() << "[MixedPathListProvider::traverse] Directory path too long or deep:" << currentDir
                        << "length:" << currentDir.size() << "depth:" << currentDir.count('/');
            continue;
        }

        // 检查目录是否已访问
        if (!IndexTraverseUtils::isValidDirectory(currentDir, visitedDirs)) {
            fmDebug() << "[MixedPathListProvider::traverse] Directory already visited or invalid:" << currentDir;
            continue;
        }

        DIR *dir = opendir(currentDir.toStdString().c_str());
        if (!dir) {
            fmWarning() << "[MixedPathListProvider::traverse] Failed to open directory:" << currentDir
                        << "error:" << strerror(errno);
            continue;
        }

        ScopeGuard dirCloser([dir]() { closedir(dir); });
        processedDirs++;

        struct dirent *entry;
        while ((entry = readdir(dir))) {
            if (!state.isRunning()) {
                fmInfo() << "[MixedPathListProvider::traverse] Directory scan interrupted";
                break;
            }

            if (IndexTraverseUtils::isSpecialDir(entry->d_name))
                continue;

            QString entryName = QString::fromUtf8(entry->d_name);
            // 检查目录名是否在黑名单中
            if (defaultBlacklistedDirs.contains(entryName)) {
                fmDebug() << "[MixedPathListProvider::traverse] Skipping blacklisted entry:" << entryName;
                continue;
            }

            QString fullPath = QDir::cleanPath(currentDir + QDir::separator() + entryName);

            struct stat st;
            if (lstat(fullPath.toStdString().c_str(), &st) == -1) {
                fmDebug() << "[MixedPathListProvider::traverse] Failed to stat entry:" << fullPath
                          << "error:" << strerror(errno);
                continue;
            }

            // 对于普通文件，早期扩展名过滤
            if (S_ISREG(st.st_mode)) {
                // 早期扩展名检查 - 如果扩展名不支持，直接跳过
                if (!IndexTraverseUtils::isSupportedFileExtension(entryName)) {
                    skippedFilesByExtension++;
                    continue;   // 跳过不支持的文件扩展名，避免后续昂贵的路径验证操作
                }

                // 只有通过扩展名检查的文件才进行昂贵的路径验证
                if (IndexTraverseUtils::isValidFile(fullPath)
                    && IndexUtility::isPathInContentIndexDirectory(fullPath)) {
                    handler(fullPath);
                    processedFiles.insert(fullPath);
                    additionalFiles++;
                }
            }
            // 对于目录，加入队列
            else if (S_ISDIR(st.st_mode)) {
                // 目录已在上面检查过黑名单，所以这里直接加入队列
                dirQueue.enqueue(fullPath);
            }
        }
    }

    fmInfo() << "[MixedPathListProvider::traverse] Traversal completed - processed directories:" << processedDirs
             << "additional files:" << additionalFiles << "total unique files:" << processedFiles.size()
             << "skipped files by extension:" << skippedFilesByExtension;
}
