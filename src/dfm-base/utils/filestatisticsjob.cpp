// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filestatisticsjob.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/private/filestatisticsjob_p.h>

#include <dfm-io/dfmio_utils.h>

#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>
#include <QStorageInfo>
#include <QElapsedTimer>
#include <QDebug>
#include <QApplication>

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
        qCWarning(logDFMBase) << "File statistics timer thread has no event loop, thread:" << notifyDataTimer->thread();
    }

    if (s == FileStatisticsJob::kRunningState) {
        QMetaObject::invokeMethod(notifyDataTimer, "start", Q_ARG(int, 500));
        elapsedTimer.start();
        qCInfo(logDFMBase) << "File statistics job started";
    } else {
        QMetaObject::invokeMethod(notifyDataTimer, "stop");

        if (s == FileStatisticsJob::kStoppedState) {
            Q_EMIT q->dataNotify(totalSize, filesCount, directoryCount);
            Q_EMIT q->sizeChanged(totalSize);
        }

        qCInfo(logDFMBase) << "File statistics job finished - total size:" << totalSize 
                          << "files:" << filesCount << "directories:" << directoryCount;
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

    processFile(info, followLink, directoryQueue);
}

void FileStatisticsJobPrivate::processFile(const FileInfoPointer &fileInfo, const bool followLink, QQueue<QUrl> &directoryQueue)
{
    auto info = fileInfo;
    auto url = fileInfo->fileUrl();

    if (!info) {
        qCWarning(logDFMBase) << "File statistics failed: unsupported URL scheme:" << url;
        return;
    }

    if (!checkInode(info))
        return;

    if (info->isAttributes(OptInfoType::kIsDir)) {
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
            if (allFiles.contains(symLinkTargetUrl) || fileStatistics.contains(symLinkTargetUrl)) {
                return;
            }
            fileStatistics << symLinkTargetUrl;
        }

        ++directoryCount;

        if (!(fileHints & (FileStatisticsJob::kDontSkipAVFSDStorage | FileStatisticsJob::kDontSkipPROCStorage)) && url.isLocalFile()) {
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

                if (!fileHints.testFlag(FileStatisticsJob::kSingleDepth) && info->isAttributes(OptInfoType::kIsDir))
                    directoryQueue << url;

            } while (false);
        } else if (!fileHints.testFlag(FileStatisticsJob::kSingleDepth) && info->isAttributes(OptInfoType::kIsDir)) {
            directoryQueue << url;
        }
    } else {
        do {
            auto isSyslink = info->isAttributes(OptInfoType::kIsSymLink);
            if (isSyslink) {
                const auto &symLinkTargetUrl = QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget));
                if (allFiles.contains(symLinkTargetUrl) || fileStatistics.contains(symLinkTargetUrl)) {
                    return;
                }
                fileStatistics << symLinkTargetUrl;
            }

            // ###(zccrs): skip the file,os file
            if (UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/proc/kcore"))
                || UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/dev/core"))) {
                break;
            }
            // skip os file Shortcut
            if (info->isAttributes(OptInfoType::kIsSymLink)
                && (skipPath.contains(info->pathOf(PathInfoType::kSymLinkTarget)))) {
                break;
            }

            const FileInfo::FileType &type = info->fileType();

            if (!checkFileType(type))
                break;

            auto size = info->size();
            if (size > 0) {
                totalSize += isSyslink ? 0 : size;
                emitSizeChanged();
            }
            // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
            // fix bug 202007010033【文件管理器】【5.1.2.10-1】【sp2】复制软连接的文件，进度条显示1%
            // 判断文件是否是链接文件
            totalProgressSize += (size <= 0 || isSyslink) ? FileUtils::getMemoryPageSize() : size;
        } while (false);

        ++filesCount;
    }
}

void FileStatisticsJobPrivate::processFile(const QUrl &url, struct stat64 *statBuffer, const bool followLink, QQueue<QUrl> &directoryQueue)
{
    if (!statBuffer)
        return;
    bool isDir = S_ISDIR(statBuffer->st_mode);
    if (!checkInode(statBuffer->st_ino, url.path()))
        return;
    if (isDir) {
        processDirectory(url, followLink, directoryQueue);
    } else {
        processRegularFile(url, statBuffer, followLink);
    }
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
        inodelist.insert(fileInode);
    }
    return true;
}

bool FileStatisticsJobPrivate::checkInode(const __ino64_t innode, const QString &path)
{
    QString key = innode > 0 ? QString::number(innode) : QString::number(innode) + ":" + path;
    if (inodeAndPath.contains(key))
        return false;

    inodeAndPath.insert(key);
    return true;
}

FileInfo::FileType FileStatisticsJobPrivate::fileType(const __mode_t fileMode)
{
    FileInfo::FileType fileType { FileInfo::FileType::kUnknown };
    if (S_ISDIR(fileMode))
        fileType = FileInfo::FileType::kDirectory;
    else if (S_ISCHR(fileMode))
        fileType = FileInfo::FileType::kCharDevice;
    else if (S_ISBLK(fileMode))
        fileType = FileInfo::FileType::kBlockDevice;
    else if (S_ISFIFO(fileMode))
        fileType = FileInfo::FileType::kFIFOFile;
    else if (S_ISSOCK(fileMode))
        fileType = FileInfo::FileType::kSocketFile;
    else if (S_ISREG(fileMode))
        fileType = FileInfo::FileType::kRegularFile;

    return fileType;
}

QString FileStatisticsJobPrivate::resolveSymlink(const QUrl &url)
{
    QSet<QString> visited;
    QString target = FileUtils::symlinkTarget(url);
    while (!target.isEmpty()) {
        if (visited.contains(target))
            return QString();   // Cycle detected: return empty
        visited.insert(target);
        QUrl newUrl = QUrl::fromLocalFile(target);
        QString nextTarget = FileUtils::symlinkTarget(newUrl);
        if (nextTarget.isEmpty())
            break;
        target = nextTarget;
    }
    return target;
}

void FileStatisticsJobPrivate::processDirectory(const QUrl &url, bool followLink, QQueue<QUrl> &directoryQueue)
{
    totalProgressSize += FileUtils::getMemoryPageSize();
    QString target = resolveSymlink(url);
    if (!target.isEmpty() && !followLink) {
        ++directoryCount;
        return;
    }
    directoryCount++;
    // Centralize storage filtering
    if (!(fileHints & (FileStatisticsJob::kDontSkipAVFSDStorage | FileStatisticsJob::kDontSkipPROCStorage))) {
        if (!fileHints.testFlag(FileStatisticsJob::kDontSkipPROCStorage) && target.startsWith("/proc"))
            return;
        if (!fileHints.testFlag(FileStatisticsJob::kDontSkipAVFSDStorage) && target.startsWith("/avfsd"))
            return;
    }
    if (!fileHints.testFlag(FileStatisticsJob::kSingleDepth))
        directoryQueue << url;
}

void FileStatisticsJobPrivate::processRegularFile(const QUrl &url, struct stat64 *statBuffer, bool followLink)
{
    QString target = resolveSymlink(url);
    bool isSymlink = !target.isEmpty();
    if (isSymlink && !followLink) {
        ++filesCount;
        return;
    }
    // Skip specific system files early
    if (UniversalUtils::urlEquals(url, QUrl::fromLocalFile("/proc/kcore")) || UniversalUtils::urlEquals(url, QUrl::fromLocalFile("/dev/core")) || target == "/proc/kcore" || target == "/dev/core") {
        return;
    }
    const FileInfo::FileType type = fileType(statBuffer->st_mode);
    if (!checkFileType(type))
        return;

    auto size = statBuffer->st_size;
    if (size > 0) {
        totalSize += isSymlink ? 0 : size;
        emitSizeChanged();
    }
    totalProgressSize += (size <= 0 || isSymlink) ? FileUtils::getMemoryPageSize() : size;
    ++filesCount;
}

FileStatisticsJob::FileStatisticsJob(QObject *parent)
    : QThread(parent), d(new FileStatisticsJobPrivate(this))
{
    d->notifyDataTimer = new QTimer(this);

    connect(
            d->notifyDataTimer, &QTimer::timeout, this, [this] {
                Q_EMIT dataNotify(d->totalSize, d->filesCount, d->directoryCount);
            },
            Qt::DirectConnection);
    connect(qApp, &QApplication::aboutToQuit, this, [this] {
        stop();   // Signal the thread to stop

        if (!wait(3000)) {   // Wait for 3 seconds
            qCWarning(logDFMBase) << "File statistics job thread did not exit within 3 seconds, terminating forcefully";
            quit();   // Ensure the event loop is stopped
            terminate();   // Forcefully terminate the thread (use with caution!)
            wait();   // Wait for the thread to terminate (no timeout this time)
        }
    });
}

FileStatisticsJob::~FileStatisticsJob()
{
    stop();
    wait();
}

FileStatisticsJob::State FileStatisticsJob::state() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return static_cast<FileStatisticsJob::State>(d->state.loadRelaxed());
#else
    return static_cast<FileStatisticsJob::State>(d->state.load());
#endif
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
        qCWarning(logDFMBase) << "File statistics job already running, rejecting new start request";
        return;
    }
    d->sourceUrlList = sourceUrls;

    if (d->sourceUrlList.count() <= 0) {
        qCWarning(logDFMBase) << "File statistics job start failed: empty source URL list";
        return;
    }

    if (d->fileHints.testFlag(kDontSizeInfoPointer))
        d->sizeInfo.reset(nullptr);

    qCInfo(logDFMBase) << "Starting file statistics job for" << sourceUrls.count() << "URLs";
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
    if (d->sourceUrlList.isEmpty())
        return;

    if (d->sourceUrlList.first().scheme() == Global::Scheme::kFile)
        return statisticsRealPathSingle();

    statistcsOtherFileSystem();
}

void FileStatisticsJob::setSizeInfo()
{
    if (d->sizeInfo.isNull())
        return;
    d->sizeInfo->fileCount = static_cast<quint32>(d->filesCount);
    d->sizeInfo->totalSize = d->totalProgressSize;
    d->sizeInfo->dirSize = FileUtils::getMemoryPageSize();
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
            if (d->allFiles.contains(url))
                continue;

            if (!d->sizeInfo.isNull())
                d->sizeInfo->allFiles << url;
            d->allFiles.insert(url);
            FileInfoPointer info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);

            if (!info) {
                qCWarning(logDFMBase) << "File statistics skipped unsupported URL:" << url;
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
                if (d->fileStatistics.contains(symLinkTargetUrl) || d->allFiles.contains(symLinkTargetUrl))
                    continue;

                info = InfoFactory::create<FileInfo>(symLinkTargetUrl, Global::CreateFileInfoType::kCreateFileInfoSync);

                if (info->isAttributes(OptInfoType::kIsSymLink)) {
                    continue;
                }

                d->fileStatistics << symLinkTargetUrl;
            }

            if (info->isAttributes(OptInfoType::kIsDir)) {
                directory_queue << url;
            }
        }
    } else {
        for (const QUrl &url : d->sourceUrlList) {
            // 选择的列表中包含avfsd/proc挂载路径时禁用过滤
            FileHints save_file_hints = d->fileHints;
            d->fileHints = d->fileHints | kDontSkipAVFSDStorage | kDontSkipPROCStorage;
            d->processFile(url, followLink, directory_queue);
            if (!d->sizeInfo.isNull())
                d->sizeInfo->allFiles << url;
            d->allFiles.insert(url);
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
            qCWarning(logDFMBase) << "File statistics failed to create directory iterator for URL:" << directory_url;
            continue;
        }
        d->iterator->setProperty("QueryAttributes", "standard::name,standard::type,standard::size,\
                                        standard::size,standard::is-symlink,standard::symlink-target,access::*,unix::inode");
        d->iteratorCanStop = true;
        while (d->iterator->hasNext()) {
            QUrl url = d->iterator->next();
            // The files counted are not counted
            if (d->allFiles.contains(url))
                continue;

            d->processFile(url, followLink, directory_queue);
            if (!d->sizeInfo.isNull())
                d->sizeInfo->allFiles << url;
            d->allFiles.insert(url);

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

void FileStatisticsJob::statisticsRealPathSingle()
{
    Q_EMIT dataNotify(0, 0, 0);

    const bool followLink = !d->fileHints.testFlag(kNoFollowSymlink);

    QQueue<QUrl> directory_queue;
    int fileCount = 0;
    if (d->fileHints.testFlag(kExcludeSourceFile)) {
        for (const QUrl &url : d->sourceUrlList) {
            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                return;
            }

            struct stat64 statBuffer;
            if (::stat64(url.path().toStdString().data(), &statBuffer) != 0)
                continue;

            bool isDir = S_ISDIR(statBuffer.st_mode);
            if (!d->checkInode(statBuffer.st_ino, url.path()))
                continue;

            if (isDir && d->fileHints.testFlag(kSingleDepth)) {
                fileCount += d->countFileCount(url.path().toStdString().data());
            } else {
                fileCount++;
            }

            if (S_ISLNK(statBuffer.st_mode)) {
                if (!followLink) {
                    continue;
                }

                const auto &symLinkTarget = FileUtils::symlinkTarget(url);
                if (::stat64(symLinkTarget.toStdString().data(), &statBuffer) != 0)
                    continue;

                isDir = S_ISDIR(statBuffer.st_mode);
                if (S_ISLNK(statBuffer.st_mode)) {
                    continue;
                }
            }

            if (isDir)
                directory_queue << url;
        }
    } else {
        for (const QUrl &url : d->sourceUrlList) {
            // 选择的列表中包含avfsd/proc挂载路径时禁用过滤
            FileHints save_file_hints = d->fileHints;
            d->fileHints = d->fileHints | kDontSkipAVFSDStorage | kDontSkipPROCStorage;
            struct stat64 statBuffer;
            if (::stat64(url.path().toStdString().data(), &statBuffer) != 0)
                continue;

            d->processFile(url, &statBuffer, followLink, directory_queue);

            if (!d->sizeInfo.isNull())
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
        if (d->fileHints.testFlag(kExcludeSourceFile))
            d->filesCount = fileCount;
        directory_queue.clear();
        setSizeInfo();
        return;
    }

    while (!directory_queue.isEmpty()) {

        if (!d->stateCheck()) {
            d->setState(kStoppedState);
            setSizeInfo();
            return;
        }

        const QUrl &directory_url = directory_queue.dequeue();

        DIR *dir { nullptr };
        struct dirent *entry { nullptr };

        if (!(dir = opendir(directory_url.path().toStdString().data())))
            continue;

        while ((entry = readdir(dir))) {
            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                closedir(dir);
                return;
            }
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            struct stat64 statBuffer;
            QString currentPath = directory_url.path() + QDir::separator() + entry->d_name;
            if (::stat64(currentPath.toStdString().data(), &statBuffer) != 0)
                continue;

            QUrl currentFile = QUrl::fromLocalFile(currentPath);

            d->processFile(currentFile, &statBuffer, followLink, directory_queue);

            if (!d->sizeInfo.isNull())
                d->sizeInfo->allFiles << currentFile;
        }
        closedir(dir);
    }
    setSizeInfo();
    d->setState(kStoppedState);
}

}
