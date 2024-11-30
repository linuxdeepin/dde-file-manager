// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indextraverseutils.h"

#include <QFileInfo>
#include <QStorageInfo>
#include <QSet>
#include <QMutex>

#include <fstab.h>
#include <sys/stat.h>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace IndexTraverseUtils {

bool isHiddenFile(const char *name)
{
    return name[0] == '.';
}

bool isSpecialDir(const char *name)
{
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

bool isValidFile(const QString &path)
{
    QFileInfo fileInfo(path);
    return !fileInfo.canonicalFilePath().isEmpty();
}

bool isValidDirectory(const QString &path, QSet<QString> &visitedDirs)
{
    QFileInfo fileInfo(path);
    QString canonicalPath = fileInfo.canonicalFilePath();

    if (canonicalPath.isEmpty())
        return false;

    if (visitedDirs.contains(canonicalPath))
        return false;

    visitedDirs.insert(canonicalPath);
    return true;
}

bool shouldSkipDirectory(const QString &path)
{
    // 基于 Baloo 和 Tracker 的经验，直接排除的系统目录
    static const QSet<QString> kExcludeDirs = {
        // 系统目录
        "/proc",   // 进程信息
        "/sys",   // 系统信息
        "/dev",   // 设备文件
        "/boot",   // 启动文件
        "/lost+found",   // 系统恢复目录
        "/tmp",   // 临时文件
        "/var/tmp",   // 临时文件
        "/var/cache",   // 缓存目录
        "/var/log",   // 日志目录
        "/var/lib",
        "/run",   // 运行时文件

        // 特殊目录
        "/opt",   // 可选软件
        "/srv",   // 服务数据
        "/usr/lib",   // 系统库
        "/usr/lib32",
        "/usr/lib64",
        "/usr/libx32",
        "/usr/share",   // 共享数据
        "/usr/include",   // 头文件
        "/usr/src",   // 源代码
        "/usr/sbin",   // 系统二进制
        "/usr/local/lib",   // 本地库
        "/usr/local/sbin",
        "/usr/local/include",
        "/usr/local/share",
        "/data"
    };

    // 检查是否在排除列表中
    for (const QString &excludeDir : kExcludeDirs) {
        if (path.startsWith(excludeDir))
            return true;
    }

    // 检查挂载点属性（参考 Tracker）
    QStorageInfo storage(path);
    if (storage.isValid()) {
        // 排除临时文件系统
        if (storage.fileSystemType() == "tmpfs" || storage.fileSystemType() == "devtmpfs" || storage.fileSystemType() == "devpts")
            return true;
    }

    return false;
}

QMap<QString, QString> fstabBindInfo()
{
    static QMutex mutex;
    static QMap<QString, QString> table;
    struct stat statInfo;
    int result = stat("/etc/fstab", &statInfo);

    QMutexLocker locker(&mutex);
    if (0 == result) {
        static quint32 lastModify = 0;
        if (lastModify != statInfo.st_mtime) {
            lastModify = static_cast<quint32>(statInfo.st_mtime);
            table.clear();
            struct fstab *fs;

            setfsent();
            while ((fs = getfsent()) != nullptr) {
                QString mntops(fs->fs_mntops);
                if (mntops.contains("bind"))
                    table.insert(fs->fs_spec, fs->fs_file);
            }
            endfsent();
        }
    }

    return table;
}

}   // namespace IndexTraverseUtils

SERVICETEXTINDEX_END_NAMESPACE
