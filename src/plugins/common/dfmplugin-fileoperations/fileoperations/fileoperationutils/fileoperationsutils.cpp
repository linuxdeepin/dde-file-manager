// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperationsutils.h"
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-io/dfmio_utils.h>

#include <QDirIterator>
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

/*!
 * \brief FileOperationsUtils::statisticsFilesSize 使用c库统计文件大小
 * 统计了所有文件的大小信息（如果文件大小 <= 0就统计这个文件大小为一个内存页，有些文件系统dir有大小，就统计dir大小，
 * 如果dir大小 <= 0就统计这个dir大小为一个内存页），统计文件的数量, 统计所有的文件及子目录路径
 * \param files 统计文件的urllist
 * \param isRecordUrl 是否统计所有的文件及子目录路径
 * \return QSharedPointer<FileOperationsUtils::FilesSizeInfo> 文件大小信息
 */
SizeInfoPointer FileOperationsUtils::statisticsFilesSize(const QList<QUrl> &files, const bool &isRecordUrl)
{
    SizeInfoPointer filesSizeInfo(new DFMBASE_NAMESPACE::FileUtils::FilesSizeInfo);
    filesSizeInfo->dirSize = FileUtils::getMemoryPageSize();

    for (auto url : files) {
        statisticFilesSize(url, filesSizeInfo, isRecordUrl);
    }

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
                                             const bool &isRecordUrl)
{
    static const QString kOriginPathPrefix = QLatin1String("originPath::");

    QSet<QUrl> urlCounted;

    char *paths[2] = { nullptr, nullptr };

    // 对无效的文件名称进行处理
    QByteArray pathData;
    if (url.userInfo().contains(kOriginPathPrefix)) {
        pathData = url.userInfo().replace(kOriginPathPrefix, "").toLatin1();
    } else {
        pathData = url.path().toUtf8();
    }

    paths[0] = strdup(pathData.constData());
    if (!paths[0]) {
        fmWarning() << "Failed to allocate memory for path";
        return;
    }

    FTS *fts = fts_open(paths, 0, nullptr);
    if (!fts) {
        perror("fts_open");
        fmWarning() << "fts_open open error : " << QString::fromLocal8Bit(strerror(errno));
        free(paths[0]);
        return;
    }

    free(paths[0]);

    while (1) {
        FTSENT *ent = fts_read(fts);
        if (ent == nullptr) {
            break;
        }
        QUrl curUrl = QUrl::fromLocalFile(ent->fts_path);
        if (DFMIO::DFMUtils::isInvalidCodecByPath(ent->fts_path))
            curUrl.setUserInfo(kOriginPathPrefix + QString::fromLatin1(ent->fts_path));
        if (urlCounted.contains(curUrl))
            continue;

        urlCounted.insert(curUrl);

        unsigned short flag = ent->fts_info;

        const auto &fileSize = ent->fts_statp->st_size;

        // url record
        if (isRecordUrl && flag != FTS_DP)
            sizeInfo->allFiles.append(curUrl);

        // file counted
        if (flag == FTS_F || flag == FTS_SL || flag == FTS_SLNONE)
            sizeInfo->fileCount++;

        // total size
        if (flag == FTS_D)
            sizeInfo->totalSize += FileUtils::getMemoryPageSize();
        else if (flag != FTS_DP)
            sizeInfo->totalSize += (fileSize > 0 ? fileSize : FileUtils::getMemoryPageSize());
    }
    fts_close(fts);
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
