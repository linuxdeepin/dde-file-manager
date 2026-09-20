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

FileSystemProvider::FileSystemProvider(IndexProfile profile, const QString &rootPath)
    : m_profile(std::move(profile)),
      m_rootPath(rootPath)
{
    // Per-profile 黑名单（filename: anything blacklist + 索引目录，避免索引器
    // 遍历进自身索引目录造成死循环/索引污染）；未提供时不过滤（Content/Ocr 行为不变，
    // 其黑名单在 TaskHandler 的 shouldSkipExcludedFile 层生效）。
    if (m_profile.filterPolicy().blacklistProvider) {
        m_excludeMatcher.addPatterns(m_profile.filterPolicy().blacklistProvider());
    }

    fmInfo() << "[FileSystemProvider] Initialized with root path:" << rootPath
             << "blacklist patterns:" << m_excludeMatcher.patternCount();
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
        if (!state.isRunning() || state.isPauseRequested()) {
            fmInfo() << "[FileSystemProvider::traverse] Traversal interrupted - running:" << state.isRunning()
                     << "pauseRequested:" << state.isPauseRequested();
            break;
        }

        QString currentPath = dirQueue.dequeue();

        // 检查是否应该跳过此目录
        if (IndexTraverseUtils::shouldSkipDirectory(currentPath)) {
            fmDebug() << "[FileSystemProvider::traverse] Skipping directory:" << currentPath;
            continue;
        }

        // 检查是否是系统目录或绑定目录
        if (!m_profile.isPathInScope(currentPath)) {
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

        // 目录自身也作为文档写入索引（filename profile，file_type=dir）。
        // 置于所有跳过检查之后、递归之前：入索引的目录集合与实际遍历的目录集合严格一致，
        // 且目录先于其子条目建档，搜索语义上父子同步可见
        if (m_profile.filterPolicy().indexDirectories)
            handler(currentPath);

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
            if (!state.isRunning() || state.isPauseRequested()) {
                fmInfo() << "[FileSystemProvider::traverse] Traversal interrupted during directory scan - pause:" << state.isPauseRequested();
                break;
            }

            if (IndexTraverseUtils::isSpecialDir(entry->d_name))
                continue;

            QString fullPath = QDir::cleanPath(currentPath + QDir::separator() + QString::fromUtf8(entry->d_name));

            // 隐藏文件策略由 profile 统一决定（IndexProfile::shouldSkipHiddenEntry）：
            // Content/Ocr 跳过所有隐藏条目；filename 索引隐藏文件（标记 is_hidden 字段），
            // 但屏蔽策略命中的条目（默认主目录下的隐藏条目，其子树同样不入遍历队列）
            if (IndexTraverseUtils::isHiddenFile(entry->d_name)
                && m_profile.shouldSkipHiddenEntry(fullPath))
                continue;

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
                if (IndexTraverseUtils::isValidFile(fullPath) && m_profile.isCandidateFile(fullPath)
                    && !m_excludeMatcher.shouldExclude(fullPath)) {
                    handler(fullPath);
                    processedFiles++;
                }
            }
            // 对于目录，检查是否应该跳过（含 per-profile 黑名单，如索引目录），然后加入队列
            else if (S_ISDIR(st.st_mode)) {
                if (!IndexTraverseUtils::shouldSkipDirectory(fullPath)
                    && !m_excludeMatcher.shouldExclude(fullPath)) {
                    dirQueue.enqueue(fullPath);
                }
            }
            // 符号链接条目（lstat 视角）：link 本身作为条目入索引（filename profile），
            // 绝不跟随——dir link 不入遍历队列（S_ISDIR 经 lstat 不会命中 link），
            // link 内部内容不经链接路径入索引。dangling link 也是文件管理器中的
            // 真实可见条目，同样收录（filename 文档全部路径派生，无需目标存在）。
            else if (S_ISLNK(st.st_mode) && m_profile.filterPolicy().indexSymlinks) {
                if (m_profile.isCandidateFile(fullPath) && !m_excludeMatcher.shouldExclude(fullPath)) {
                    handler(fullPath);
                    processedFiles++;
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
        if (!state.isRunning() || state.isPauseRequested()) {
            fmInfo() << "[DirectFileListProvider::traverse] Processing interrupted after" << processedCount << "files - pause:" << state.isPauseRequested();
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

QStringList DirectFileListProvider::filePaths() const
{
    QStringList paths;
    paths.reserve(m_fileList.size());
    for (const auto &result : std::as_const(m_fileList))
        paths.append(result.path());
    return paths;
}

MixedPathListProvider::MixedPathListProvider(IndexProfile profile, const QStringList &pathList)
    : m_profile(std::move(profile)),
      m_pathList(pathList)
{
    // Per-profile 黑名单（filename: anything blacklist + 索引目录，死循环防护）；
    // 未提供时不过滤（Content/Ocr 维持原有 folderExcludeFilters 逻辑）
    if (m_profile.filterPolicy().blacklistProvider) {
        m_excludeMatcher.addPatterns(m_profile.filterPolicy().blacklistProvider());
    }

    fmInfo() << "[MixedPathListProvider] Initialized with" << pathList.size() << "paths,"
             << "profile blacklist patterns:" << m_excludeMatcher.patternCount();
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
        if (!state.isRunning() || state.isPauseRequested()) {
            fmInfo() << "[MixedPathListProvider::traverse] Initial processing interrupted - pause:" << state.isPauseRequested();
            break;
        }

        QFileInfo fileInfo(path);
        // 与 FileSystemProvider 同构的隐藏条目过滤（防御事件之外来源的路径，
        // 如 DBus 手动触发）：策略命中的隐藏条目不建档、不入遍历队列
        if (m_profile.shouldSkipHiddenEntry(path)) {
            fmDebug() << "[MixedPathListProvider::traverse] Skipping hidden entry per profile policy:" << path;
            continue;
        }

        // 符号链接条目（lstat 视角，filename profile）：link 本身作为条目建档，
        // 不按目标类型分流——dangling link 的 exists()/isFile() 均为 false，
        // 必须在 exists 检查之前识别；dir link 绝不入遍历队列。
        if (fileInfo.isSymLink()) {
            if (m_profile.filterPolicy().indexSymlinks
                && m_profile.isPathInScope(path)
                && m_profile.isCandidateFile(path)
                && !m_excludeMatcher.shouldExclude(path)
                && !processedFiles.contains(path)) {
                handler(path);
                processedFiles.insert(path);
                initialFiles++;
            }
            continue;
        }

        if (!fileInfo.exists()) {
            fmWarning() << "[MixedPathListProvider::traverse] Path does not exist:" << path;
            continue;
        }

        if (fileInfo.isFile()) {
            if (IndexTraverseUtils::isValidFile(path)
                && m_profile.isPathInScope(path)
                && m_profile.isCandidateFile(path)) {
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
            // 检查目录是否在黑名单中（TextIndexConfig 目录名 + per-profile 黑名单路径）
            bool isBlacklisted = false;
            QString dirName = fileInfo.fileName();
            if (defaultBlacklistedDirs.contains(dirName) || m_excludeMatcher.shouldExclude(path)) {
                isBlacklisted = true;
                fmDebug() << "[MixedPathListProvider::traverse] Directory blacklisted:" << path;
            }

            // 只有不在黑名单中的目录才加入队列
            if (!isBlacklisted) {
                // 目录自身建档（filename profile）：与上方文件分支同构，
                // 同样受 scope 约束，避免范围外路径混入索引
                if (m_profile.filterPolicy().indexDirectories
                    && m_profile.isPathInScope(path)) {
                    handler(path);
                }
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
        if (!state.isRunning() || state.isPauseRequested()) {
            fmInfo() << "[MixedPathListProvider::traverse] Directory traversal interrupted - pause:" << state.isPauseRequested();
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

        // 目录自身建档（filename profile），与递归集合一致（同 FileSystemProvider）
        if (m_profile.filterPolicy().indexDirectories)
            handler(currentDir);

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
            if (!state.isRunning() || state.isPauseRequested()) {
                fmInfo() << "[MixedPathListProvider::traverse] Directory scan interrupted - pause:" << state.isPauseRequested();
                break;
            }

            if (IndexTraverseUtils::isSpecialDir(entry->d_name))
                continue;

            QString entryName = QString::fromUtf8(entry->d_name);
            // 检查目录名是否在黑名单中（TextIndexConfig 目录名）
            if (defaultBlacklistedDirs.contains(entryName)) {
                fmDebug() << "[MixedPathListProvider::traverse] Skipping blacklisted entry:" << entryName;
                continue;
            }

            QString fullPath = QDir::cleanPath(currentDir + QDir::separator() + entryName);

            // 与 FileSystemProvider 同构的隐藏条目过滤（策略命中的条目不建档、不入队）
            if (IndexTraverseUtils::isHiddenFile(entry->d_name)
                && m_profile.shouldSkipHiddenEntry(fullPath))
                continue;

            struct stat st;
            if (lstat(fullPath.toStdString().c_str(), &st) == -1) {
                fmDebug() << "[MixedPathListProvider::traverse] Failed to stat entry:" << fullPath
                          << "error:" << strerror(errno);
                continue;
            }

            // 对于普通文件，早期扩展名过滤
            if (S_ISREG(st.st_mode)) {
                if (IndexTraverseUtils::isValidFile(fullPath)
                    && m_profile.isPathInScope(fullPath)
                    && m_profile.isCandidateFile(fullPath)
                    && !m_excludeMatcher.shouldExclude(fullPath)) {
                    handler(fullPath);
                    processedFiles.insert(fullPath);
                    additionalFiles++;
                } else {
                    skippedFilesByExtension++;
                }
            }
            // 对于目录，加入队列（per-profile 黑名单目录不遍历）
            else if (S_ISDIR(st.st_mode)) {
                if (!m_excludeMatcher.shouldExclude(fullPath)) {
                    dirQueue.enqueue(fullPath);
                }
            }
            // 符号链接条目（filename profile）：link 本身作为条目入索引，
            // 绝不跟随/入队（dir link 内部内容不经链接路径索引）
            else if (S_ISLNK(st.st_mode) && m_profile.filterPolicy().indexSymlinks) {
                if (m_profile.isPathInScope(fullPath)
                    && m_profile.isCandidateFile(fullPath)
                    && !m_excludeMatcher.shouldExclude(fullPath)
                    && !processedFiles.contains(fullPath)) {
                    handler(fullPath);
                    processedFiles.insert(fullPath);
                    additionalFiles++;
                }
            }
        }
    }

    fmInfo() << "[MixedPathListProvider::traverse] Traversal completed - processed directories:" << processedDirs
             << "additional files:" << additionalFiles << "total unique files:" << processedFiles.size()
             << "skipped files by extension:" << skippedFilesByExtension;
}
