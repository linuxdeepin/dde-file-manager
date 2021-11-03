/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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
#include "dfilestatisticsjob.h"
#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "dstorageinfo.h"
#include "shutil/fileutils.h"
#include <models/trashfileinfo.h>

#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>


DFM_BEGIN_NAMESPACE

class DFileStatisticsJobPrivate
{
public:
    DFileStatisticsJobPrivate(DFileStatisticsJob *qq);
    ~DFileStatisticsJobPrivate();

    void setState(DFileStatisticsJob::State s);

    bool jobWait();
    bool stateCheck();

    void processFile(const DUrl &url, QQueue<DUrl> &directoryQueue);

    DFileStatisticsJob *q_ptr;
    QTimer *notifyDataTimer;

    QAtomicInt state = DFileStatisticsJob::StoppedState;
    DFileStatisticsJob::FileHints fileHints;

    DUrlList sourceUrlList;
    QWaitCondition waitCondition;

    QAtomicInteger<qint64> totalSize = 0;
    // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
    QAtomicInteger<qint64> totalProgressSize = 0;
    QAtomicInt filesCount = 0;
    QAtomicInt directoryCount = 0;
};

DFileStatisticsJobPrivate::DFileStatisticsJobPrivate(DFileStatisticsJob *qq)
    : q_ptr(qq)
    , notifyDataTimer(nullptr)
{

}

DFileStatisticsJobPrivate::~DFileStatisticsJobPrivate()
{
    if (notifyDataTimer) {
        notifyDataTimer->stop();
        notifyDataTimer->deleteLater();
    }
}

void DFileStatisticsJobPrivate::setState(DFileStatisticsJob::State s)
{
    if (s == state) {
        return;
    }

    state = s;

    if (notifyDataTimer->thread() && notifyDataTimer->thread()->loopLevel() <= 0) {
        qWarning() << "The thread of notify data timer no event loop" << notifyDataTimer->thread();
    }

    if (s == DFileStatisticsJob::RunningState) {
        QMetaObject::invokeMethod(notifyDataTimer, "start", Q_ARG(int, 500));
    } else {
        QMetaObject::invokeMethod(notifyDataTimer, "stop");

        if (s == DFileStatisticsJob::StoppedState) {
            Q_EMIT q_ptr->dataNotify(totalSize, filesCount, directoryCount);
        }
    }

    Q_EMIT q_ptr->stateChanged(s);
}

bool DFileStatisticsJobPrivate::jobWait()
{
    QMutex lock;

    lock.lock();
    waitCondition.wait(&lock);
    lock.unlock();

    return state == DFileStatisticsJob::RunningState;
}

bool DFileStatisticsJobPrivate::stateCheck()
{
    if (state == DFileStatisticsJob::RunningState) {
        return true;
    }

    if (state == DFileStatisticsJob::PausedState) {
        if (!jobWait()) {
            return false;
        }
    } else if (state == DFileStatisticsJob::StoppedState) {
        return false;
    }

    return true;
}

void DFileStatisticsJobPrivate::processFile(const DUrl &url, QQueue<DUrl> &directoryQueue)
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);

    if (!info) {
        qDebug() << "Url not yet supported: " << url;
        return;
    }

    qint64 size = 0;

    if (info->isFile()) {
        do {
            // ###(zccrs): skip the file,os file
            if (info->fileUrl() == DUrl::fromLocalFile("/proc/kcore") || info->fileUrl() == DUrl::fromLocalFile("/dev/core")) {
                break;
            }
            //skip os file Shortcut
            if (info->isSymLink() && (info->symlinkTargetPath() == QStringLiteral("/proc/kcore") || info->symlinkTargetPath() == QStringLiteral("/dev/core")))
            {
                break;
            }

            const DAbstractFileInfo::FileType type = info->fileType();

            if (type == DAbstractFileInfo::CharDevice && !fileHints.testFlag(DFileStatisticsJob::DontSkipCharDeviceFile)) {
                break;
            }

            if (type == DAbstractFileInfo::BlockDevice && !fileHints.testFlag(DFileStatisticsJob::DontSkipBlockDeviceFile)) {
                break;
            }

            if (type == DAbstractFileInfo::FIFOFile && !fileHints.testFlag(DFileStatisticsJob::DontSkipFIFOFile)) {
                break;
            }

            if (type == DAbstractFileInfo::SocketFile && !fileHints.testFlag(DFileStatisticsJob::DontSkipSocketFile)) {
                break;
            }
            if (type == DAbstractFileInfo::Unknown)
            {
                break;
            }

            size = info->size();
            if (size > 0) {
                totalSize += size;

                Q_EMIT q_ptr->sizeChanged(totalSize);
            }
            // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
            // fix bug 202007010033【文件管理器】【5.1.2.10-1】【sp2】复制软连接的文件，进度条显示1%
            // 判断文件是否是链接文件
            totalProgressSize += (size <= 0 || info->isSymLink()) ? FileUtils::getMemoryPageSize() : size;
        } while (false);

        ++filesCount;

        Q_EMIT q_ptr->fileFound(url);
    } else {
        // fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
        totalProgressSize += FileUtils::getMemoryPageSize();
        if (info->isSymLink()) {
            if (!fileHints.testFlag(DFileStatisticsJob::FollowSymlink)) {
                ++filesCount;
                Q_EMIT q_ptr->fileFound(url);
                return;
            }

            info = DFileService::instance()->createFileInfo(nullptr, info->rootSymLinkTarget());

            if (info->isSymLink()) {
                ++filesCount;
                Q_EMIT q_ptr->fileFound(url);
                return;
            }
        }

//        size = info->size();
        ++directoryCount;

        if (!(fileHints & (DFileStatisticsJob::DontSkipAVFSDStorage | DFileStatisticsJob::DontSkipPROCStorage)) && info->fileUrl().isLocalFile()) {
            do {
                DStorageInfo si(info->fileUrl().toLocalFile());

                if (si.rootPath() == info->fileUrl().toLocalFile()) {
                    if (!fileHints.testFlag(DFileStatisticsJob::DontSkipPROCStorage)
                            && si.device() == "proc") {
                        break;
                    }

                    if (!fileHints.testFlag(DFileStatisticsJob::DontSkipAVFSDStorage)
                            && si.device() == "avfsd") {
                        break;
                    }
                }

                if (!fileHints.testFlag(DFileStatisticsJob::SingleDepth))
                    directoryQueue << url;
            } while (false);
        } else if (!fileHints.testFlag(DFileStatisticsJob::SingleDepth)) {
            directoryQueue << url;
        }

        Q_EMIT q_ptr->directoryFound(url);
    }

}

DFileStatisticsJob::DFileStatisticsJob(QObject *parent)
    : QThread(parent)
    , d_ptr(new DFileStatisticsJobPrivate(this))
{
    d_ptr->notifyDataTimer = new QTimer(this);

    connect(d_ptr->notifyDataTimer, &QTimer::timeout, this, [this] {
        Q_EMIT dataNotify(d_ptr->totalSize, d_ptr->filesCount, d_ptr->directoryCount);
    }, Qt::DirectConnection);
}

DFileStatisticsJob::~DFileStatisticsJob()
{
    stop();
    wait();
}

DFileStatisticsJob::State DFileStatisticsJob::state() const
{
    Q_D(const DFileStatisticsJob);

    return static_cast<DFileStatisticsJob::State>(d->state.load());
}

DFileStatisticsJob::FileHints DFileStatisticsJob::fileHints() const
{
    Q_D(const DFileStatisticsJob);

    return d->fileHints;
}

qint64 DFileStatisticsJob::totalSize() const
{
    Q_D(const DFileStatisticsJob);

    return d->totalSize.load();
}
// fix bug 30548 ,以为有些文件大小为0,文件夹为空，size也为零，重新计算显示大小
qint64 DFileStatisticsJob::totalProgressSize() const
{
    Q_D(const DFileStatisticsJob);

    return d->totalProgressSize;
}

int DFileStatisticsJob::filesCount() const
{
    Q_D(const DFileStatisticsJob);

    return d->filesCount.load();
}

int DFileStatisticsJob::directorysCount(bool includeSelf) const
{
    Q_D(const DFileStatisticsJob);

    if (includeSelf) {
        return d->directoryCount.load();
    } else {
        return qMax(d->directoryCount.load() - 1, 0);
    }
}

void DFileStatisticsJob::start(const DUrlList &sourceUrls)
{
    if (isRunning()) {
        qDebug() << "current thread is running... reject to start.";
        return;
    }
    Q_D(DFileStatisticsJob);
    d->sourceUrlList = sourceUrls;

    if (d->sourceUrlList.count() <= 0) {
        return;
    }
    //fix bug 35044 【文件管理器】【5.1.2-1】【sp2】我的共享和标记栏目下，预览文件夹，文件大小和文件个数显示错误
    // 传入的scheme为USERSHARE_SCHEME设置为FILE_SCHEME,传入的scheme是TAG_SCHEME判断taggedLocalFilePath是否为空，
    //判断path()是否为“/”,(因为这两个在tagcontroller中处理了)设置为FILE_SCHEME，设置setScheme为FILE_SCHEME，url为taggedLocalFilePath
    QList<DUrl>::iterator it = d->sourceUrlList.begin();
    while(it != d->sourceUrlList.end())
    {
        if (it->scheme() == USERSHARE_SCHEME) {
            it->setScheme(FILE_SCHEME);
        }
        if (it->scheme() == TAG_SCHEME && it->path() != QString("/") && !it->taggedLocalFilePath().isEmpty()) {
            it->setUrl(it->taggedLocalFilePath());
            it->setScheme(FILE_SCHEME);
        }
        if (it->scheme() == SEARCH_SCHEME && !it->fragment().isEmpty()) {
            it->setUrl(it->fragment());
        }
        it++;
    }

    QThread::start();
}

void DFileStatisticsJob::stop()
{
    Q_D(DFileStatisticsJob);

    if (d->state == StoppedState) {
        return;
    }

    d->setState(StoppedState);
    d->waitCondition.wakeAll();
}

void DFileStatisticsJob::togglePause()
{
    Q_D(DFileStatisticsJob);

    if (d->state == StoppedState) {
        return;
    }

    if (d->state == PausedState) {
        d->setState(RunningState);
        d->waitCondition.wakeAll();
    } else {
        d->setState(PausedState);
    }
}

void DFileStatisticsJob::setFileHints(FileHints fileHints)
{
    Q_D(DFileStatisticsJob);
    Q_ASSERT(d->state != RunningState);

    d->fileHints = fileHints;
}

void DFileStatisticsJob::run()
{
    Q_D(DFileStatisticsJob);

    d->setState(RunningState);
    d_ptr->totalSize = 0;
    d_ptr->filesCount = 0;
    d_ptr->directoryCount = 0;

    Q_EMIT dataNotify(0, 0, 0);

    QQueue<DUrl> directory_queue;
    int fileCount = 0;
    if (d->fileHints.testFlag(ExcludeSourceFile)) {
        for (const DUrl &url : d->sourceUrlList) {
            if (!d->stateCheck()) {
                d->setState(StoppedState);

                return;
            }

            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);

            if (!info) {
                qDebug() << "Url not yet supported: " << url;
                continue;
            }

            if (info->isDir() && d->fileHints.testFlag(SingleDepth)) {
                fileCount += info->filesCount();
            } else {
                fileCount++;
            }

            if (info->isSymLink()) {
                if (!d->fileHints.testFlag(DFileStatisticsJob::FollowSymlink)) {
                    continue;
                }

                info = DFileService::instance()->createFileInfo(nullptr, info->rootSymLinkTarget());

                if (info->isSymLink()) {
                    continue;
                }
            }

            if (info->isDir()) {
                directory_queue << url;
            }
        }
    } else {
        for (const DUrl &url : d->sourceUrlList) {
            // 选择的列表中包含avfsd/proc挂载路径时禁用过滤
            FileHints save_file_hints = d->fileHints;
            d->fileHints = d->fileHints | DontSkipAVFSDStorage | DontSkipPROCStorage;
            d->processFile(url, directory_queue);
            d->fileHints = save_file_hints;

            if (!d->stateCheck()) {
                d->setState(StoppedState);

                return;
            }
        }
    }

    if (d->fileHints.testFlag(SingleDepth)) {
        d->filesCount = fileCount;
        directory_queue.clear();
        return;
    }

    while (!directory_queue.isEmpty()) {
        const DUrl &directory_url = directory_queue.dequeue();
        const DDirIteratorPointer &iterator = DFileService::instance()->createDirIterator(nullptr, directory_url, QStringList(),
                                              QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, nullptr, true);

        if (!iterator) {
            qWarning() << "Failed on create dir iterator, for url:" << directory_url;
            continue;
        }
        while (iterator->hasNext()) {
            DUrl url = iterator->next();
            d->processFile(url, directory_queue);

            if (!d->stateCheck()) {
                d->setState(StoppedState);

                return;
            }
        }
    }

    d->setState(StoppedState);
}

DFM_END_NAMESPACE
