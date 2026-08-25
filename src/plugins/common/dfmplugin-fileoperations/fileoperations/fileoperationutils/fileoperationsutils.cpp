// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperationsutils.h"
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-io/dfmio_utils.h>

#include <QDirIterator>
#include <QSet>
#include <QUrl>
#include <QDebug>
#include <QMutexLocker>

#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

#include <sys/vfs.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>
#include <sys/utsname.h>

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

inline constexpr char kFileOperations[] { "org.deepin.dde.file-manager.operations" };
inline constexpr char kFileBigSize[] { "file.operation.bigfilesize" };
inline constexpr char kBlockEverySync[] { "file.operation.blockeverysync" };
inline constexpr char kBroadcastPaste[] { "file.operation.broadcastpastevent" };
inline constexpr char kUseFtsDelete[] { "file.operation.useftsdelete" };

/*!
 * \brief FileOperationsUtils::statisticsFilesSize 使用c库统计文件大小
 * 统计了所有文件的大小信息（如果文件大小 <= 0就统计这个文件大小为一个内存页，有些文件系统dir有大小，就统计dir大小，
 * 如果dir大小 <= 0就统计这个dir大小为一个内存页），统计文件的数量, 统计所有的文件及子目录路径
 * \param files 统计文件的urllist
 * \param isRecordUrl 是否统计所有的文件及子目录路径
 * \return QSharedPointer<FileOperationsUtils::FilesSizeInfo> 文件大小信息
 */

/*!
 * \brief collectFtsEntries  Common FTS traversal logic shared by
 *        statisticsFilesSize() (batch) and statisticFilesSize() (single URL).
 *
 * Opens fts_open on the given path array, walks the tree, and accumulates
 * file count, total size, and optionally all file URLs into \a sizeInfo.
 * Deduplicates overlapping source paths via a QSet.
 *
 * \param paths     NULL-terminated array of C strings (as required by fts_open)
 * \param ftsFlags  flags passed to fts_open (e.g. FTS_PHYSICAL | FTS_NOCHDIR [ | FTS_NOSTAT])
 * \param sizeInfo  accumulator (modified in place)
 * \param isRecordUrl  whether to record every URL into sizeInfo->allFiles
 * \param noStat    whether FTS_NOSTAT was set (skip size accumulation)
 */
static void collectFtsEntries(char *const *paths, int ftsFlags,
                              SizeInfoPointer &sizeInfo,
                              const bool &isRecordUrl, const bool noStat)
{
    FTS *fts = fts_open(paths, ftsFlags, nullptr);
    if (!fts) {
        fmWarning() << "collectFtsEntries: fts_open failed:" << strerror(errno);
        return;
    }

    const qint64 pageSize = FileUtils::getMemoryPageSize();

    // Dedup: fts_open does not dedup overlapping source paths, so entries reachable
    // via more than one source URL would be counted multiple times.
    QSet<QUrl> urlCounted;
    FTSENT *ent;
    while ((ent = fts_read(fts)) != nullptr) {
        unsigned short flag = ent->fts_info;

        // Handle errors / unstatable entries
        if (flag == FTS_DNR || flag == FTS_ERR || flag == FTS_DC || flag == FTS_NS) {
            fmWarning() << "collectFtsEntries: traversal error:" << ent->fts_path
                        << strerror(ent->fts_errno);
            continue;
        }
        // Post-order dir: children already processed, skip
        if (flag == FTS_DP)
            continue;

        QUrl curUrl = QUrl::fromLocalFile(ent->fts_path);
        if (DFMIO::DFMUtils::isInvalidCodecByPath(ent->fts_path))
            curUrl.setUserInfo(QString::fromLatin1("originPath::") + QString::fromLatin1(ent->fts_path));

        // Skip entries already counted via an overlapping source path.
        if (urlCounted.contains(curUrl))
            continue;
        urlCounted.insert(curUrl);

        // url record
        if (isRecordUrl)
            sizeInfo->allFiles.append(curUrl);

        // file counted — include FTS_NSOK to match deleteFilesByFts() (FTS_NOSTAT
        // returns FTS_NSOK instead of FTS_F for regular files when d_type is available).
        if (flag == FTS_F || flag == FTS_SL || flag == FTS_SLNONE || flag == FTS_NSOK)
            sizeInfo->fileCount++;

        if (noStat)
            continue;

        // total size
        if (flag == FTS_D) {
            sizeInfo->totalSize += pageSize;
        } else {
            const qint64 fileSize = ent->fts_statp->st_size;
            sizeInfo->totalSize += (fileSize > 0 ? fileSize : pageSize);
        }
    }

    fts_close(fts);
}

SizeInfoPointer FileOperationsUtils::statisticsFilesSize(const QList<QUrl> &files, const bool &isRecordUrl,
                                                         const bool noStat)
{
    SizeInfoPointer filesSizeInfo(new DFMBASE_NAMESPACE::FileUtils::FilesSizeInfo);
    const qint64 pageSize = FileUtils::getMemoryPageSize();
    filesSizeInfo->dirSize = pageSize;

    if (files.isEmpty())
        return filesSizeInfo;

    // Build path array for batch fts_open (handles originPath encoding)
    QList<QByteArray> pathData;
    pathData.reserve(files.size());
    for (const auto &url : files) {
        if (!url.isLocalFile())
            continue;
        if (url.userInfo().contains("originPath::"))
            pathData.append(url.userInfo().replace("originPath::", "").toLatin1());
        else
            pathData.append(url.path().toUtf8());
    }
    if (pathData.isEmpty())
        return filesSizeInfo;

    QVector<char *> pathPtrs(pathData.size() + 1, nullptr);
    for (int i = 0; i < pathData.size(); ++i)
        pathPtrs[i] = pathData[i].data();

    auto flags = FTS_PHYSICAL | FTS_NOCHDIR;
    if (noStat)
        flags |= FTS_NOSTAT;

    collectFtsEntries(pathPtrs.data(), flags, filesSizeInfo, isRecordUrl, noStat);
    return filesSizeInfo;
}

bool FileOperationsUtils::isFilesSizeOutLimit(const QUrl &url, const qint64 limitSize)
{
    qint64 totalSize = 0;
    char *paths[2] = { nullptr, nullptr };
    paths[0] = strdup(url.path().toUtf8().toStdString().data());
    FTS *fts = fts_open(paths, 0, nullptr);
    if (paths[0])
        free(paths[0]);

    if (nullptr == fts) {
        perror("fts_open");
        fmWarning() << "fts_open open error : " << QString::fromLocal8Bit(strerror(errno));
        return false;
    }
    while (1) {
        FTSENT *ent = fts_read(fts);
        if (ent == nullptr) {
            break;
        }
        unsigned short flag = ent->fts_info;

        if (flag != FTS_DP)
            totalSize += ent->fts_statp->st_size <= 0 ? FileUtils::getMemoryPageSize() : ent->fts_statp->st_size;

        if (totalSize > limitSize)
            break;
    }

    fts_close(fts);

    return totalSize > limitSize;
}

void FileOperationsUtils::statisticFilesSize(const QUrl &url,
                                             SizeInfoPointer &sizeInfo,
                                             const bool &isRecordUrl, const bool noStat)
{
    // Build path string with originPath encoding support
    QByteArray pathBytes;
    if (url.userInfo().contains("originPath::"))
        pathBytes = url.userInfo().replace("originPath::", "").toLatin1();
    else
        pathBytes = url.path().toUtf8();

    char *paths[2] = { strdup(pathBytes.constData()), nullptr };

    if (!paths[0]) {
        fmWarning() << "statisticFilesSize: strdup failed for" << url;
        return;
    }

    auto flags = FTS_PHYSICAL | FTS_NOCHDIR;
    if (noStat)
        flags |= FTS_NOSTAT;

    collectFtsEntries(paths, flags, sizeInfo, isRecordUrl, noStat);
    free(paths[0]);
}

bool FileOperationsUtils::isAncestorUrl(const QUrl &from, const QUrl &to)
{
    QUrl parentUrl = UrlRoute::urlParent(to);
    return from.path() == parentUrl.path();
}

bool FileOperationsUtils::isFileOnDisk(const QUrl &url)
{
    if (!url.isValid())
        return false;

    g_autoptr(GFile) destDirFile = g_file_new_for_uri(url.toString().toLocal8Bit().data());
    g_autoptr(GMount) destDirMount = g_file_find_enclosing_mount(destDirFile, nullptr, nullptr);
    if (destDirMount) {
        return !g_mount_can_unmount(destDirMount);
    }
    return true;
}

qint64 FileOperationsUtils::bigFileSize()
{
    // 获取当前配置
    qint64 bigSize = DConfigManager::instance()->value(kFileOperations, kFileBigSize).toLongLong();
    if (bigSize <= 0)
        return 80 * 1024 * 1024;
    return bigSize;
}

bool FileOperationsUtils::blockSync()
{
    bool sync = DConfigManager::instance()->value(kFileOperations, kBlockEverySync).toBool();
    return sync;
}

QUrl FileOperationsUtils::parentUrl(const QUrl &url)
{
    auto parent = url.adjusted(QUrl::StripTrailingSlash);
    parent = parent.adjusted(QUrl::RemoveFilename);
    parent = parent.adjusted(QUrl::StripTrailingSlash);
    if (parent.isParentOf(url))
        return parent;

    return QUrl();
}

bool FileOperationsUtils::canBroadcastPaste()
{
    // 组策略中配置
    return DConfigManager::instance()->value(kFileOperations, kBroadcastPaste, false).toBool();
}

bool FileOperationsUtils::useFtsDelete()
{
    return DConfigManager::instance()->value(kFileOperations, kUseFtsDelete, true).toBool();
}
