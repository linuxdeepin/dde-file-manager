/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "filestatisticsjob.h"
#include "interfaces/abstractfileinfo.h"
#include "base/schemefactory.h"
#include "interfaces/abstractdiriterator.h"

#include "dfm-base/utils/universalutils.h"

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

namespace dfmbase {

static constexpr uint16_t kSizeChangeinterval { 200 };

class FileStatisticsJobPrivate : public QObject
{
public:
    explicit FileStatisticsJobPrivate(FileStatisticsJob *qq);
    ~FileStatisticsJobPrivate();

    void setState(FileStatisticsJob::State s);

    bool jobWait();
    bool stateCheck();

    void processFile(const QUrl &url, const bool followLink, QQueue<QUrl> &directoryQueue);
    void emitSizeChanged();

    FileStatisticsJob *q;
    QTimer *notifyDataTimer;

    QAtomicInt state = FileStatisticsJob::kStoppedState;
    FileStatisticsJob::FileHints fileHints;

    QList<QUrl> sourceUrlList;
    QWaitCondition waitCondition;
    QElapsedTimer elapsedTimer;

    QAtomicInteger<qint64> totalSize = { 0 };
    QAtomicInteger<qint64> totalProgressSize { 0 };
    QAtomicInt filesCount { 0 };
    QAtomicInt directoryCount { 0 };
    SizeInfoPointer sizeInfo { nullptr };
    bool isExtFileSystem { true };
};

FileStatisticsJobPrivate::FileStatisticsJobPrivate(FileStatisticsJob *qq)
    : QObject(nullptr), q(qq), notifyDataTimer(nullptr)
{
    sizeInfo.reset(new FileUtils::FilesSizeInfo());
    sizeInfo->dirSize = FileUtils::getMemoryPageSize();
}

FileStatisticsJobPrivate::~FileStatisticsJobPrivate()
{
    if (notifyDataTimer) {
        notifyDataTimer->stop();
        notifyDataTimer->deleteLater();
    }
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
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

    if (!info) {
        qDebug() << "Url not yet supported: " << url;
        return;
    }

    qint64 size = 0;

    if (info->isAttributes(OptInfoType::kIsFile)) {
        do {
            // ###(zccrs): skip the file,os file
            if (UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/proc/kcore"))
                || UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), QUrl::fromLocalFile("/dev/core"))) {
                break;
            }
            //skip os file Shortcut
            if (info->isAttributes(OptInfoType::kIsSymLink)
                && (info->pathOf(PathInfoType::kSymLinkTarget)
                            == QStringLiteral("/proc/kcore")
                    || info->pathOf(PathInfoType::kSymLinkTarget) == QStringLiteral("/dev/core"))) {
                break;
            }

            const AbstractFileInfo::FileType type = info->fileType();

            if (type == AbstractFileInfo::FileType::kCharDevice && !fileHints.testFlag(FileStatisticsJob::kDontSkipCharDeviceFile)) {
                break;
            }

            if (type == AbstractFileInfo::FileType::kBlockDevice && !fileHints.testFlag(FileStatisticsJob::kDontSkipBlockDeviceFile)) {
                break;
            }

            if (type == AbstractFileInfo::FileType::kFIFOFile && !fileHints.testFlag(FileStatisticsJob::kDontSkipFIFOFile)) {
                break;
            }

            if (type == AbstractFileInfo::FileType::kSocketFile && !fileHints.testFlag(FileStatisticsJob::kDontSkipSocketFile)) {
                break;
            }
            if (type == AbstractFileInfo::FileType::kUnknown) {
                break;
            }

            size = info->size();
            if (size > 0) {
                totalSize += size;
                emitSizeChanged();
            }
            // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
            // fix bug 202007010033【文件管理器】【5.1.2.10-1】【sp2】复制软连接的文件，进度条显示1%
            // 判断文件是否是链接文件
            totalProgressSize += (size <= 0 || info->isAttributes(OptInfoType::kIsSymLink)) ? FileUtils::getMemoryPageSize() : size;
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
                info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget)));
            } while (info && info->isAttributes(OptInfoType::kIsSymLink));

            if (!info) {
                ++directoryCount;
                return;
            }
        }

        ++directoryCount;

        if (!(fileHints & (FileStatisticsJob::kDontSkipAVFSDStorage | FileStatisticsJob::kDontSkipPROCStorage)) && info->urlOf(UrlInfoType::kUrl).isLocalFile()) {
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

void FileStatisticsJobPrivate::emitSizeChanged()
{
    if (elapsedTimer.elapsed() > kSizeChangeinterval) {
        Q_EMIT q->sizeChanged(totalSize);
        elapsedTimer.restart();
    }
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
    return d->totalSize.load();
}
// fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
qint64 FileStatisticsJob::totalProgressSize() const
{
    return d->totalProgressSize;
}

int FileStatisticsJob::filesCount() const
{
    return d->filesCount.load();
}

int FileStatisticsJob::directorysCount(bool includeSelf) const
{
    if (includeSelf) {
        return d->directoryCount.load();
    } else {
        return qMax(d->directoryCount.load() - 1, 0);
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
    if (!d->sourceUrlList.isEmpty()) {
        const QUrl &urlFirst = d->sourceUrlList.first();
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(urlFirst);
        d->isExtFileSystem = fsType.startsWith("ext");
    }

    if (d->isExtFileSystem) {
        statistcsExtFileSystem();
    } else {
        statistcsOtherFileSystem();
    }
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
            d->sizeInfo->allFiles << url;
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);

            if (!info) {
                qDebug() << "Url not yet supported: " << url;
                continue;
            }

            if (info->isAttributes(OptInfoType::kIsDir) && d->fileHints.testFlag(kSingleDepth)) {
                fileCount += info->countChildFile();
            } else {
                fileCount++;
            }

            if (info->isAttributes(OptInfoType::kIsSymLink)) {
                if (!followLink) {
                    continue;
                }

                info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(info->pathOf(PathInfoType::kSymLinkTarget)));

                if (info->isAttributes(OptInfoType::kIsSymLink)) {
                    continue;
                }
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
            d->sizeInfo->allFiles << url;
            d->processFile(url, followLink, directory_queue);
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
        const AbstractDirIteratorPointer &iterator = DirIteratorFactory::create<AbstractDirIterator>(directory_url, QStringList(),
                                                                                                     QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);

        if (!iterator) {
            qWarning() << "Failed on create dir iterator, for url:" << directory_url;
            continue;
        }
        while (iterator->hasNext()) {
            QUrl url = iterator->next();
            d->sizeInfo->allFiles << url;
            d->processFile(url, followLink, directory_queue);

            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                return;
            }
        }
    }
    setSizeInfo();
    d->setState(kStoppedState);
}

void FileStatisticsJob::statistcsExtFileSystem()
{
    const bool excludeSources = d->fileHints.testFlag(kExcludeSourceFile);
    const bool singleDepth = d->fileHints.testFlag(kSingleDepth);
    const bool followLink = !d->fileHints.testFlag(kNoFollowSymlink);

    for (auto url : d->sourceUrlList) {
        if (!d->stateCheck()) {
            d->setState(kStoppedState);
            return;
        }
        char *paths[2] = { nullptr, nullptr };
        paths[0] = strdup(url.path().toUtf8().toStdString().data());
        FTS *fts = fts_open(paths, followLink ? FTS_LOGICAL : FTS_PHYSICAL, nullptr);
        if (paths[0])
            free(paths[0]);

        while (1) {
            // 停止就退出
            if (!d->stateCheck()) {
                d->setState(kStoppedState);
                setSizeInfo();
                break;
            }

            FTSENT *ent = fts_read(fts);
            if (ent == nullptr) {
                break;
            }

            if (excludeSources) {
                const QUrl &currentUrl = QUrl::fromLocalFile(QString::fromLocal8Bit(ent->fts_path));
                if (url == currentUrl)
                    continue;
            }

            const short level = ent->fts_level;

            if (singleDepth && level == 1) {
                int ret = fts_set(fts, ent, FTS_SKIP);
                if (-1 == ret)
                    qWarning() << "skip sub dir failed, current url: " << ent->fts_path;
            }

            unsigned short flag = ent->fts_info;

            if (level > 1 && (flag == FTS_SL || flag == FTS_SLNONE))
                continue;

            if (flag == FTS_F || flag == FTS_SL || flag == FTS_SLNONE || flag == FTS_DEFAULT) {
                // file counted
                d->filesCount++;
            } else if (flag == FTS_D || flag == FTS_DNR) {
                // dir counted
                d->directoryCount++;
            } else if (flag == FTS_DC) {
                // cycle folder
                continue;
            } else if (flag != FTS_DP) {
                // other error
                continue;
            }

            // total size
            if (flag == FTS_D) {
                d->totalSize += FileUtils::getMemoryPageSize();
                d->totalProgressSize += FileUtils::getMemoryPageSize();
                d->emitSizeChanged();
            } else if (flag != FTS_DP) {
                d->totalSize += ent->fts_statp->st_size;
                d->totalProgressSize += ent->fts_statp->st_size;
                d->emitSizeChanged();
            }
        }
        setSizeInfo();
        fts_close(fts);
    }
    d->setState(kStoppedState);
}

}
