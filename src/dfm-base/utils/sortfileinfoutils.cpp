// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sortfileinfoutils.h"

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/dfm_log_defines.h>

#include <QDir>
#include <QFile>
#include <QUrl>

#include <fcntl.h>
#include <linux/stat.h>
#include <sys/stat.h>
#include <unistd.h>

DFMBASE_BEGIN_NAMESPACE

namespace SortInfoUtils {

/**
 * @brief 解析符号链接目标路径
 * @param entryPath  符号链接自身路径
 * @param parentPath 符号链接所在父目录路径
 * @return 解析后的目标绝对路径，失败返回空字符串
 */
static QString resolveSymlinkTargetPath(const QString &entryPath, const QString &parentPath)
{
    QByteArray buffer;
    buffer.resize(PATH_MAX);
    const QByteArray nativePath = QFile::encodeName(entryPath);
    const ssize_t size = ::readlink(nativePath.constData(), buffer.data(), buffer.size() - 1);
    if (size <= 0)
        return QString();

    buffer[static_cast<size_t>(size)] = '\0';
    QString targetPath = QFile::decodeName(buffer.constData());
    if (QDir::isRelativePath(targetPath))
        targetPath = QDir(parentPath).absoluteFilePath(targetPath);

    return QDir::cleanPath(targetPath);
}

QSet<QString> loadHideFileList(const QString &dirPath)
{
    QFile hiddenFile(QDir(dirPath).filePath(".hidden"));
    if (!hiddenFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    const QString data = QString::fromUtf8(hiddenFile.readAll());
    const QStringList entries = data.split('\n', Qt::SkipEmptyParts);
    return QSet<QString>(entries.begin(), entries.end());
}

SortInfoPointer createSortInfo(const QString &parentPath,
                               const QString &fileName,
                               const QSet<QString> &hideList)
{
    const QString entryPath = QDir(parentPath).filePath(fileName);
    const QByteArray nativePath = QFile::encodeName(entryPath);

    // 使用 statx 获取所有文件属性（包括创建时间 birth time）
    struct statx stx;
    unsigned int mask = STATX_BASIC_STATS | STATX_BTIME;
    if (statx(AT_FDCWD, nativePath.constData(), AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT, mask, &stx) != 0) {
        qCWarning(logDFMBase) << "createSortInfo: statx failed for" << entryPath << ":" << strerror(errno);
        return nullptr;
    }

    const bool isSymLink = S_ISLNK(stx.stx_mode);
    mode_t effectiveMode = stx.stx_mode;
    uint64_t effectiveSize = stx.stx_size;
    time_t effectiveAtime = stx.stx_atime.tv_sec;
    time_t effectiveMtime = stx.stx_mtime.tv_sec;
    time_t effectiveBtime = (stx.stx_mask & STATX_BTIME) ? stx.stx_btime.tv_sec : 0;

    // 符号链接：获取目标文件属性
    if (isSymLink) {
        const QString targetPath = resolveSymlinkTargetPath(entryPath, parentPath);
        if (!targetPath.isEmpty() && !ProtocolUtils::isRemoteFile(QUrl::fromLocalFile(targetPath))) {
            const QByteArray targetNativePath = QFile::encodeName(targetPath);
            struct statx targetStx;
            if (statx(AT_FDCWD, targetNativePath.constData(), AT_SYMLINK_NOFOLLOW | AT_NO_AUTOMOUNT, mask, &targetStx) == 0) {
                effectiveMode = targetStx.stx_mode;
                effectiveSize = targetStx.stx_size;
                effectiveAtime = targetStx.stx_atime.tv_sec;
                effectiveMtime = targetStx.stx_mtime.tv_sec;
                if (targetStx.stx_mask & STATX_BTIME)
                    effectiveBtime = targetStx.stx_btime.tv_sec;
            } else {
                qCWarning(logDFMBase) << "createSortInfo: statx failed for symlink target" << targetPath << ":" << strerror(errno);
            }
        }
    }

    SortInfoPointer info(new SortFileInfo);
    info->setUrl(QUrl::fromLocalFile(entryPath));
    info->setSize(effectiveSize);
    info->setSymlink(isSymLink);
    info->setDir(S_ISDIR(effectiveMode));
    info->setFile(!S_ISDIR(effectiveMode));
    info->setHide(fileName.startsWith(".") || hideList.contains(fileName));
    info->setReadable((effectiveMode & S_IRUSR) != 0);
    info->setWriteable((effectiveMode & S_IWUSR) != 0);
    info->setExecutable((effectiveMode & S_IXUSR) != 0);
    info->setLastReadTime(effectiveAtime);
    info->setLastModifiedTime(effectiveMtime);
    info->setCreateTime(effectiveBtime);
    info->setInfoCompleted(true);
    return info;
}

}   // namespace SortInfoUtils

DFMBASE_END_NAMESPACE
