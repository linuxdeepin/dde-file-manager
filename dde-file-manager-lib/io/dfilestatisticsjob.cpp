/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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

#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>

DFM_BEGIN_NAMESPACE

class DFileStatisticsJobPrivate
{
public:
    DFileStatisticsJobPrivate(DFileStatisticsJob *qq);

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
    QAtomicInt filesCount = 0;
    QAtomicInt directoryCount = 0;
};

DFileStatisticsJobPrivate::DFileStatisticsJobPrivate(DFileStatisticsJob *qq)
    : q_ptr(qq)
{

}

void DFileStatisticsJobPrivate::setState(DFileStatisticsJob::State s)
{
    if (s == state) {
        return;
    }

    state = s;

    if (notifyDataTimer->thread()->loopLevel() <= 0) {
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

    qint64 size = 0;

    if (info->isFile()) {
        do {
            // ###(zccrs): skip the file
            if (info->fileUrl() == DUrl::fromLocalFile("/proc/kcore")) {
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

            size = info->size();
        } while (false);

        ++filesCount;

        Q_EMIT q_ptr->fileFound(url);
    } else {
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

    if (size > 0) {
        totalSize += size;
        Q_EMIT q_ptr->sizeChanged(totalSize);
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

int DFileStatisticsJob::filesCount() const
{
    Q_D(const DFileStatisticsJob);

    return d->filesCount.load();
}

int DFileStatisticsJob::directorysCount() const
{
    Q_D(const DFileStatisticsJob);

    return d->directoryCount.load();
}

void DFileStatisticsJob::start(const DUrlList &sourceUrls)
{
    Q_ASSERT(!isRunning());
    Q_D(DFileStatisticsJob);

    d->sourceUrlList = sourceUrls;
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

    while (!directory_queue.isEmpty()) {
        const DUrl &directory_url = directory_queue.dequeue();
        const DDirIteratorPointer &iterator = DFileService::instance()->createDirIterator(nullptr, directory_url, QStringList(),
                                              QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, 0, true);

        if (!iterator) {
            qWarning() << "Failed on create dir iterator, for url:" << directory_url;
            continue;
        }

        while (iterator->hasNext()) {
            d->processFile(iterator->next(), directory_queue);

            if (!d->stateCheck()) {
                d->setState(StoppedState);

                return;
            }
        }
    }

    d->setState(StoppedState);
}

DFM_END_NAMESPACE
