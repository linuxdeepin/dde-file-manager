// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileprovider.h"
#include "utils/indextraverseutils.h"
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
