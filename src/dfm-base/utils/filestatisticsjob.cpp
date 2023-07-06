// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filestatisticsjob.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/private/filestatissticsjob_p.h>

#include <dfm-io/dfmio_utils.h>

#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>
#include <QStorageInfo>
#include <QElapsedTimer>
#include <QDebug>

#include <fts.h>
#include <sys/stat.h>
#include <dirent.h>

namespace dfmbase {

static constexpr uint16_t kSizeChangeinterval { 200 };

FileStatisticsJobPrivate::FileStatisticsJobPrivate(FileStatisticsJob *qq)
    : QObject(nullptr), q(qq), notifyDataTimer(nullptr)
{
    sizeInfo.reset(new FileUtils::FilesSizeInfo());
    sizeInfo->dirSize = FileUtils::getMemoryPageSize();
    skipPath << "/proc/kcore"
             << "/dev/core";
}

FileStatisticsJobPrivate::~FileStatisticsJobPrivate()
{
    if (notifyDataTimer) {
        notifyDataTimer->stop();
        notifyDataTimer->deleteLater();
    }
    inodelist.clear();
}

void FileStatisticsJobPrivate::setState(FileStatisticsJob::State s)
{
    if (s == state) {
        return;
    }

    state = s;

    if (notifyDataTimer->thread() && notifyDataTimer->thread()->loopLevel() <= 0) {
        qWarning() << "The thread of notify data timer no event loop" << notifyDataTimer->thread();
    }

    if (s == FileStatisticsJob::kRunningState) {
        QMetaObject::invokeMethod(notifyDataTimer, "start", Q_ARG(int, 500));
        elapsedTimer.start();
    } else {
        QMetaObject::invokeMethod(notifyDataTimer, "stop");

        if (s == FileStatisticsJob::kStoppedState) {
            Q_EMIT q->dataNotify(totalSize, filesCount, directoryCount);
            Q_EMIT q->sizeChanged(totalSize);
        }

        qDebug() << "statistic finished(may stop), result: " << totalSize << filesCount << directoryCount;
    }

    Q_EMIT q->stateChanged(s);
}

bool FileStatisticsJobPrivate::jobWait()
{
    QMutex lock;

    lock.lock();
    waitCondition.wait(&lock);
    lock.unlock();

    return state == FileStatisticsJob::kRunningState;
}

bool FileStatisticsJobPrivate::stateCheck()
{
    if (state == FileStatisticsJob::kRunningState) {
        return true;
    }

    if (state == FileStatisticsJob::kPausedState) {
        if (!jobWait()) {
            return false;
        }
    } else if (state == FileStatisticsJob::kStoppedState) {
        return false;
    }

    return true;
}

void FileStatisticsJobPrivate::processFile(const QUrl &url, const bool followLink, QQueue<QUrl> &directoryQueue)
{
    FileInfoPointer info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);

    if (!info) {
        qDebug() << "Url not yet supported: " << url;
        return;
    }

    if (!checkInode(info))
        return;

    if (info->isAttributes(OptInfoType::kIsFile)) {
        do {
            auto isSyslink = info->isAttributes(OptInfoType::kIsSymLink);
            if (isSyslink) {
                const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
                if (sizeInfo->allFiles.contains(symLinkTargetUrl) || fileStatistics.contains(symLinkTargetUrl)) {
                    return;
                }
                fileStatistics << symLinkTargetUrl;
            }

            // ###(zccrs): skip the file,os file
            if (UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/proc/kcore"))
                || UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/dev/core"))) {
                break;
            }
            //skip os file Shortcut
            if (info->isAttributes(OptInfoType::kIsSymLink)
                && (skipPath.contains(info->pathOf(PathInfoType::kSymLinkTarget)))) {
                break;
            }

            const FileInfo::FileType &type = info->fileType();

            if (!checkFileType(type))
                break;

            auto size = info->size();
            if (size > 0) {
                totalSize += size;
                emitSizeChanged();
            }
            // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
            // fix bug 202007010033【文件管理器】【5.1.2.10-1】【sp2】复制软连接的文件，进度条显示1%
            // 判断文件是否是链接文件
            totalProgressSize += (size <= 0 || isSyslink) ? FileUtils::getMemoryPageSize() : size;
        } while (false);

        ++filesCount;
    } else {
        // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
        totalProgressSize += FileUtils::getMemoryPageSize();
        if (info->isAttributes(OptInfoType::kIsSymLink)) {
            if (!followLink) {
                ++directoryCount;
                return;
            }

            do {
                info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget)),
                                                     Global::CreateFileInfoType::kCreateFileInfoSync);
            } while (info && info->isAttributes(OptInfoType::kIsSymLink));

            if (!info) {
                ++directoryCount;
                return;
            }

            const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
            if (sizeInfo->allFiles.contains(symLinkTargetUrl) || fileStatistics.contains(symLinkTargetUrl)) {
                return;
            }
            fileStatistics << symLinkTargetUrl;
        }

        ++directoryCount;

        if (!(fileHints & (FileStatisticsJob::kDontSkipAVFSDStorage | FileStatisticsJob::kDontSkipPROCStorage)) && dfmbase::FileUtils::isLocalFile(info->urlOf(UrlInfoType::kUrl))) {
            do {
                QStorageInfo si(info->urlOf(UrlInfoType::kUrl).toLocalFile());

                if (si.rootPath() == info->urlOf(UrlInfoType::kUrl).toLocalFile()) {
                    if (!fileHints.testFlag(FileStatisticsJob::kDontSkipPROCStorage)
                        && si.device() == "proc") {
                        break;
                    }

                    if (!fileHints.testFlag(FileStatisticsJob::kDontSkipAVFSDStorage)
                        && si.device() == "avfsd") {
                        break;
                    }
                }

                if (!fileHints.testFlag(FileStatisticsJob::kSingleDepth))
                    directoryQueue << url;
            } while (false);
        } else if (!fileHints.testFlag(FileStatisticsJob::kSingleDepth)) {
            directoryQueue << url;
        }
    }
}

void FileStatisticsJobPrivate::processFileByFts(const QUrl &url, const bool followLink)
{
    // The files counted are not counted
    if (sizeInfo->allFiles.contains(url))
        return;

    char *paths[2] = { nullptr, nullptr };
    paths[0] = strdup(url.path().toUtf8().toStdString().data());
    auto openflags = fileHints.testFlag(FileStatisticsJob::kExcludeSourceFile) ? FTS_COMFOLLOW : 0;
    FTS *fts = fts_open(paths, openflags, nullptr);

    if (nullptr == fts) {
        qWarning() << "open file by fts failed ! url = " << url << ", case " << strerror(errno);
        return;
    }
    if (paths[0])
        free(paths[0]);

    auto singleDepth = fileHints.testFlag(FileStatisticsJob::kSingleDepth);
    while (1) {
        FTSENT *ent = fts_read(fts);
        if (nullptr == ent)
            break;

        if (!stateCheck())
            break;

        if (!checkInode(ent, fts))
            continue;

        QUrl currentUrl = QUrl::fromLocalFile(ent->fts_path);

        if (fileHints.testFlag(FileStatisticsJob::kExcludeSourceFile) && currentUrl.path() == url.path())
            continue;

        sizeInfo->allFiles.append(currentUrl);

        auto isLink = S_ISLNK(ent->fts_statp->st_mode);
        if (isLink) {
            statisticSysLink(currentUrl, fts, ent, singleDepth, followLink);
            continue;
        }

        if (skipPath.contains(ent->fts_path)) {
            filesCount++;
            continue;
        }

        if (!S_ISDIR(ent->fts_statp->st_mode)) {
            statisticFile(ent);
            continue;
        }

        statisticDir(url, fts, singleDepth, ent);
    }

    if (fts)
        fts_close(fts);
}

void FileStatisticsJobPrivate::emitSizeChanged()
{
    if (elapsedTimer.elapsed() > kSizeChangeinterval) {
        Q_EMIT q->sizeChanged(totalSize);
        elapsedTimer.restart();
    }
}

int FileStatisticsJobPrivate::countFileCount(const char *name)
{
    if (strlen(name) >= FILENAME_MAX)
        return 0;

    DIR *dir { nullptr };
    struct dirent *entry { nullptr };
    int fileCount = 0;

    if (!(dir = opendir(name)))
        return fileCount;

    while ((entry = readdir(dir))) {
        if (!stateCheck())
            break;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        fileCount++;
    }

    closedir(dir);
    return fileCount;
}

bool FileStatisticsJobPrivate::checkFileType(const FileInfo::FileType &fileType)
{
    if (fileType == FileInfo::FileType::kCharDevice && !fileHints.testFlag(FileStatisticsJob::kDontSkipCharDeviceFile)) {
        return false;
    }

    if (fileType == FileInfo::FileType::kBlockDevice && !fileHints.testFlag(FileStatisticsJob::kDontSkipBlockDeviceFile)) {
        return false;
    }

    if (fileType == FileInfo::FileType::kFIFOFile && !fileHints.testFlag(FileStatisticsJob::kDontSkipFIFOFile)) {
        return false;
    }

    if (fileType == FileInfo::FileType::kSocketFile && !fileHints.testFlag(FileStatisticsJob::kDontSkipSocketFile)) {
        return false;
    }

    if (fileType == FileInfo::FileType::kUnknown) {
        return false;
    }

    return true;
}

FileInfo::FileType FileStatisticsJobPrivate::getFileType(const uint mode)
{
    FileInfo::FileType fileType { FileInfo::FileType::kUnknown };
    if (S_ISDIR(mode))
        fileType = FileInfo::FileType(FileInfo::FileType::kDirectory);
    else if (S_ISCHR(mode))
        fileType = FileInfo::FileType(FileInfo::FileType::kCharDevice);
    else if (S_ISBLK(mode))
        fileType = FileInfo::FileType(FileInfo::FileType::kBlockDevice);
    else if (S_ISFIFO(mode))
        fileType = FileInfo::FileType(FileInfo::FileType::kFIFOFile);
    else if (S_ISSOCK(mode))
        fileType = FileInfo::FileType(FileInfo::FileType::kSocketFile);
    else if (S_ISREG(mode))
        fileType = FileInfo::FileType(FileInfo::FileType::kRegularFile);

    return fileType;
}

void FileStatisticsJobPrivate::statisticDir(const QUrl &url, FTS *fts, const bool singleDepth, FTSENT *ent)
{
    if (sizeInfo->dirSize == 0) {
        sizeInfo->dirSize = ent->fts_statp->st_size == 0
                ? FileUtils::getMemoryPageSize()
                : static_cast<quint16>(ent->fts_statp->st_size);
    }
    totalProgressSize += FileUtils::getMemoryPageSize();
    ++directoryCount;
    if (singleDepth && QString(ent->fts_path) != url.path())
        fts_set(fts, ent, FTS_SKIP);
}

void FileStatisticsJobPrivate::statisticFile(FTSENT *ent)
{
    const FileInfo::FileType &fileType = getFileType(ent->fts_statp->st_mode);
    if (!checkFileType(fileType))
        return;
    filesCount++;
    totalSize += ent->fts_statp->st_size;
    totalProgressSize += ent->fts_statp->st_size <= 0 ? FileUtils::getMemoryPageSize() : ent->fts_statp->st_size;
}

void FileStatisticsJobPrivate::statisticSysLink(const QUrl &currentUrl, FTS *fts, FTSENT *ent, const bool singleDepth, const bool followLink)
{
    if (!singleDepth) {
        if (S_ISDIR(ent->fts_statp->st_mode)) {
            directoryCount++;
        } else {
            filesCount++;
        }
        totalSize += ent->fts_statp->st_size;
        return;
    }
    auto info = InfoFactory::create<FileInfo>(currentUrl, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!info) {
        filesCount++;
        return;
    }
    const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
    if (sizeInfo->allFiles.contains(symLinkTargetUrl) || fileStatistics.contains(symLinkTargetUrl)) {
        return;
    }
    if (skipPath.contains(symLinkTargetUrl.path())) {
        filesCount++;
        return;
    }

    if (info->isAttributes(OptInfoType::kIsFile)) {
        const FileInfo::FileType &type = info->fileType();
        if (!checkFileType(type))
            return;

        totalProgressSize += FileUtils::getMemoryPageSize();
        fileStatistics << symLinkTargetUrl;
        filesCount++;
        totalSize += info->size();
        return;
    }

    ++directoryCount;
    if (singleDepth)
        return;

    fileStatistics << symLinkTargetUrl;
    if (followLink)
        fts_set(fts, ent, FTS_SYMFOLLOW);
}

bool FileStatisticsJobPrivate::checkInode(const FileInfoPointer info)
{
    auto fileInode = info->extendAttributes(ExtInfoType::kInode).toULongLong();
    if (fileInode > 0) {
        if (inodelist.contains(fileInode)) {
            if (info->isAttributes(OptInfoType::kIsFile)) {
                filesCount++;
            } else {
                directoryCount++;
            }
            return false;
        }
        inodelist.append(fileInode);
    }
    return true;
}

bool FileStatisticsJobPrivate::checkInode(FTSENT *ent, FTS *fts)
{
    auto fileInode =  ent->fts_statp->st_ino;
    if (fileInode > 0) {
        if (inodelist.contains(fileInode)) {
            if (S_ISDIR(ent->fts_statp->st_mode))
                fts_set(fts, ent, FTS_SKIP);
            return false;
        }
        inodelist.append(fileInode);
    }
    return true;
}

FileStatisticsJob::FileStatisticsJob(QObject *parent)
    : QThread(parent), d(new FileStatisticsJobPrivate(this))
{
    d->notifyDataTimer = new QTimer(this);

    connect(d->notifyDataTimer, &QTimer::timeout, this, [this] {
        Q_EMIT dataNotify(d->totalSize, d->filesCount, d->directoryCount);
    },
            Qt::DirectConnection);
}

FileStatisticsJob::~FileStatisticsJob()
{
    stop();
    wait();
}

FileStatisticsJob::State FileStatisticsJob::state() const
{
    return static_cast<FileStatisticsJob::State>(d->state.load());
}

FileStatisticsJob::FileHints FileStatisticsJob::fileHints() const
{
    return d->fileHints;
}

qint64 FileStatisticsJob::totalSize() const
{
    return d->totalSize;
}
// fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
qint64 FileStatisticsJob::totalProgressSize() const
{
    return d->totalProgressSize;
}

int FileStatisticsJob::filesCount() const
{
    return d->filesCount;
}

int FileStatisticsJob::directorysCount(bool includeSelf) const
{
    if (includeSelf) {
        return d->directoryCount;
    } else {
        return qMax(d->directoryCount - 1, 0);
    }
}

SizeInfoPointer FileStatisticsJob::getFileSizeInfo()
{
    return d->sizeInfo;
}

void FileStatisticsJob::start(const QList<QUrl> &sourceUrls)
{
    if (isRunning()) {
        qDebug() << "current thread is running... reject to start.";
        return;
    }
    d->sourceUrlList = sourceUrls;

    if (d->sourceUrlList.count() <= 0) {
        return;
    }

    QThread::start();
}

void FileStatisticsJob::stop()
{
    if (d->iterator && d->iteratorCanStop)
        d->iterator->close();

    if (d->state == kStoppedState) {
        return;
    }

    d->setState(kStoppedState);
    d->waitCondition.wakeAll();
}

void FileStatisticsJob::togglePause()
{

    if (d->state == kStoppedState) {
        return;
    }

    if (d->state == kPausedState) {
        d->setState(kRunningState);
        d->waitCondition.wakeAll();
    } else {
        d->setState(kPausedState);
    }
}

void FileStatisticsJob::setFileHints(FileHints fileHints)
{
    Q_ASSERT(d->state != kRunningState);

    d->fileHints = fileHints;
}

void FileStatisticsJob::run()
{
    d->setState(kRunningState);
    d->totalSize = 0;
    d->filesCount = 0;
    d->directoryCount = 0;
    d->inodelist.clear();
    d->sizeInfo.reset(new FileUtils::FilesSizeInfo());
    if (d->sourceUrlList.isEmpty())
        return;
    if (!FileUtils::isLocalDevice(d->sourceUrlList.first())) {
        statistcsOtherFileSystem();
        return;
    }
    statistcsByFts();
}

void FileStatisticsJob::setSizeInfo()
{
    d->sizeInfo->fileCount = static_cast<quint32>(d->filesCount);
    d->sizeInfo->totalSize = d->totalProgressSize;
    d->sizeInfo->dirSize = d->sizeInfo->dirSize == 0 ? FileUtils::getMemoryPageSize() : d->sizeInfo->dirSize;
}

void FileStatisticsJob::statistcsOtherFileSystem()
{
    Q_EMIT dataNotify(0, 0, 0);

    const bool followLink = !d->fileHints.testFlag(kNoFollowSymlink);

    QQueue<QUrl> directory_queue;
    int fileCount = 0;
    if (d->fileHints.testFlag(kExcludeSourceFile)) {
        for (const QUrl &url : d->sourceUrlList) {
            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                return;
            }
            // The files counted are not counted
            if (d->sizeInfo->allFiles.contains(url))
                continue;

            d->sizeInfo->allFiles << url;
            FileInfoPointer info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);

            if (!info) {
                qDebug() << "Url not yet supported: " << url;
                continue;
            }

            if (!d->checkInode(info))
                continue;

            if (info->isAttributes(OptInfoType::kIsDir) && d->fileHints.testFlag(kSingleDepth)) {
                fileCount += d->countFileCount(info->pathOf(PathInfoType::kAbsoluteFilePath).toStdString().c_str());
            } else {
                fileCount++;
            }

            if (info->isAttributes(OptInfoType::kIsSymLink)) {
                if (!followLink) {
                    continue;
                }

                const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
                // The files counted are not counted
                if (d->fileStatistics.contains(symLinkTargetUrl) || d->sizeInfo->allFiles.contains(symLinkTargetUrl))
                    continue;

                info = InfoFactory::create<FileInfo>(symLinkTargetUrl, Global::CreateFileInfoType::kCreateFileInfoSync);

                if (info->isAttributes(OptInfoType::kIsSymLink)) {
                    continue;
                }

                d->fileStatistics << symLinkTargetUrl;
            }

            if (info->isAttributes(OptInfoType::kIsDir)) {
                if (d->sizeInfo->dirSize == 0) {
                    struct stat statInfo;

                    if (0 == stat(info->urlOf(UrlInfoType::kUrl).path().toStdString().data(), &statInfo))
                        d->sizeInfo->dirSize = statInfo.st_size == 0 ? FileUtils::getMemoryPageSize() : static_cast<quint16>(statInfo.st_size);
                }
                directory_queue << url;
            }
        }
    } else {
        for (const QUrl &url : d->sourceUrlList) {
            // 选择的列表中包含avfsd/proc挂载路径时禁用过滤
            FileHints save_file_hints = d->fileHints;
            d->fileHints = d->fileHints | kDontSkipAVFSDStorage | kDontSkipPROCStorage;
            d->processFile(url, followLink, directory_queue);
            d->sizeInfo->allFiles << url;
            d->fileHints = save_file_hints;

            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                return;
            }
        }
    }

    if (d->fileHints.testFlag(kSingleDepth)) {
        d->filesCount = fileCount;
        directory_queue.clear();
        setSizeInfo();
        return;
    }

    while (!directory_queue.isEmpty()) {
        const QUrl &directory_url = directory_queue.dequeue();
        d->iterator = DirIteratorFactory::create<AbstractDirIterator>(directory_url, QStringList(),
                                                                      QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

        if (!d->iterator) {
            qWarning() << "Failed on create dir iterator, for url:" << directory_url;
            continue;
        }
        d->iteratorCanStop = true;
        while (d->iterator->hasNext()) {
            QUrl url = d->iterator->next();
            // The files counted are not counted
            if (d->sizeInfo->allFiles.contains(url))
                continue;

            d->processFile(url, followLink, directory_queue);
            d->sizeInfo->allFiles << url;

            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                return;
            }
        }
        d->iteratorCanStop = false;
    }
    setSizeInfo();
    d->setState(kStoppedState);
}

void FileStatisticsJob::statistcsByFts()
{
    Q_EMIT dataNotify(0, 0, 0);

    const bool followLink = !d->fileHints.testFlag(kNoFollowSymlink);

    for (const auto &url : d->sourceUrlList) {
        if (!d->stateCheck()) {
            d->setState(kStoppedState);
            setSizeInfo();
            return;
        }

        d->processFileByFts(url, followLink);
    }
    d->setState(kStoppedState);
}

}
