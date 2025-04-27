// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileprovider.h"
#include "utils/indextraverseutils.h"
#include "utils/indexutility.h"
#include "utils/scopeguard.h"

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
}

void FileSystemProvider::traverse(TaskState &state, const FileHandler &handler)
{
    QMap<QString, QString> bindPathTable = IndexTraverseUtils::fstabBindInfo();
    QSet<QString> visitedDirs;
    QQueue<QString> dirQueue;
    dirQueue.enqueue(m_rootPath);

    while (!dirQueue.isEmpty()) {
        if (!state.isRunning())
            break;

        QString currentPath = dirQueue.dequeue();

        // 检查是否是系统目录或绑定目录
        if (bindPathTable.contains(currentPath) || IndexTraverseUtils::shouldSkipDirectory(currentPath))
            continue;

        // 检查路径长度和深度限制
        if (currentPath.size() > FILENAME_MAX - 1 || currentPath.count('/') > 20)
            continue;

        // 检查目录是否已访问
        if (!IndexTraverseUtils::isValidDirectory(currentPath, visitedDirs))
            continue;

        DIR *dir = opendir(currentPath.toStdString().c_str());
        if (!dir) {
            fmWarning() << "Cannot open directory:" << currentPath;
            continue;
        }

        ScopeGuard dirCloser([dir]() { closedir(dir); });

        struct dirent *entry;
        while ((entry = readdir(dir))) {
            if (!state.isRunning())
                break;

            if (IndexTraverseUtils::isHiddenFile(entry->d_name) || IndexTraverseUtils::isSpecialDir(entry->d_name))
                continue;

            QString fullPath = QDir::cleanPath(currentPath + QDir::separator() + QString::fromUtf8(entry->d_name));

            struct stat st;
            if (lstat(fullPath.toStdString().c_str(), &st) == -1)
                continue;

            // 对于普通文件，只检查路径有效性
            if (S_ISREG(st.st_mode)) {
                if (IndexTraverseUtils::isValidFile(fullPath)) {
                    handler(fullPath);
                }
            }
            // 对于目录，加入队列（后续会检查是否访问过）
            else if (S_ISDIR(st.st_mode)) {
                dirQueue.enqueue(fullPath);
            }
        }
    }
}

DirectFileListProvider::DirectFileListProvider(const dfmsearch::SearchResultList &files)
    : m_fileList(files)
{
}

void DirectFileListProvider::traverse(TaskState &state, const FileHandler &handler)
{
    for (const auto &file : std::as_const(m_fileList)) {
        if (!state.isRunning())
            break;
        handler(file.path());
    }
}

MixedPathListProvider::MixedPathListProvider(const QStringList &pathList)
    : m_pathList(pathList)
{
}

void MixedPathListProvider::traverse(TaskState &state, const FileHandler &handler)
{
    // TODO (search): use dconfig
    // Default blacklisted directories
    static QStringList kDefaultBlacklistedDirs = {
        ".git", ".svn", ".hg", ".cache", ".local/share/Trash", ".Trash",
        ".thumbnails", "thumbnails", ".mozilla"
    };

    QSet<QString> processedFiles;   // 避免重复处理文件
    QSet<QString> visitedDirs;   // 避免目录循环引用

    // 首先处理列表中的文件和准备目录遍历
    QQueue<QString> dirQueue;

    for (const auto &path : std::as_const(m_pathList)) {
        if (!state.isRunning())
            break;

        QFileInfo fileInfo(path);
        if (!fileInfo.exists())
            continue;

        if (fileInfo.isFile()) {
            // 处理文件
            if (IndexTraverseUtils::isValidFile(path)
                && IndexUtility::isPathInContentIndexDirectory(path)) {
                handler(path);
                processedFiles.insert(path);
            }
        } else if (fileInfo.isDir() && !fileInfo.isSymLink()) {
            // 检查目录是否在黑名单中
            bool isBlacklisted = false;
            QString dirName = fileInfo.fileName();
            if (kDefaultBlacklistedDirs.contains(dirName)) {
                isBlacklisted = true;
            }

            // 只有不在黑名单中的目录才加入队列
            if (!isBlacklisted) {
                dirQueue.enqueue(path);
            }
        }
    }

    // 处理所有目录
    QMap<QString, QString> bindPathTable = IndexTraverseUtils::fstabBindInfo();

    while (!dirQueue.isEmpty()) {
        if (!state.isRunning())
            break;

        QString currentDir = dirQueue.dequeue();

        // 检查是否是系统目录或绑定目录
        if (bindPathTable.contains(currentDir) || IndexTraverseUtils::shouldSkipDirectory(currentDir))
            continue;

        // 检查路径长度和深度限制
        if (currentDir.size() > FILENAME_MAX - 1 || currentDir.count('/') > 20)
            continue;

        // 检查目录是否已访问
        if (!IndexTraverseUtils::isValidDirectory(currentDir, visitedDirs))
            continue;

        DIR *dir = opendir(currentDir.toStdString().c_str());
        if (!dir) {
            fmWarning() << "Cannot open directory:" << currentDir;
            continue;
        }

        ScopeGuard dirCloser([dir]() { closedir(dir); });

        struct dirent *entry;
        while ((entry = readdir(dir))) {
            if (!state.isRunning())
                break;

            if (IndexTraverseUtils::isSpecialDir(entry->d_name))
                continue;

            QString entryName = QString::fromUtf8(entry->d_name);
            // 检查目录名是否在黑名单中
            if (kDefaultBlacklistedDirs.contains(entryName))
                continue;

            QString fullPath = QDir::cleanPath(currentDir + QDir::separator() + entryName);

            struct stat st;
            if (lstat(fullPath.toStdString().c_str(), &st) == -1)
                continue;

            // 对于普通文件，只处理未处理过的
            if (S_ISREG(st.st_mode)) {
                if (IndexTraverseUtils::isValidFile(fullPath)
                    && IndexUtility::isPathInContentIndexDirectory(fullPath)) {
                    handler(fullPath);
                    processedFiles.insert(fullPath);
                }
            }
            // 对于目录，加入队列
            else if (S_ISDIR(st.st_mode)) {
                // 目录已在上面检查过黑名单，所以这里直接加入队列
                dirQueue.enqueue(fullPath);
            }
        }
    }
}
