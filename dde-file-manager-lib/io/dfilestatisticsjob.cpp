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

#include <QMutex>
#include <QQueue>
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
    if (s == state)
        return;

    state = s;

    Q_EMIT q_ptr->stateChanged(s);
}

bool DFileStatisticsJobPrivate::jobWait()
{
    QMutex lock;

    lock.lock();
    waitCondition.wait(&lock);

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
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(q_ptr, url);

    totalSize += info->size();

    if (info->isFile() || info->isSymLink()) {
        ++filesCount;

        Q_EMIT q_ptr->fileFound(info->fileUrl());
    } else {
        ++directoryCount;
        directoryQueue << url;

        Q_EMIT q_ptr->directoryFound(info->fileUrl());
    }

    Q_EMIT q_ptr->sizeChanged(totalSize);
}

DFileStatisticsJob::DFileStatisticsJob(QObject *parent)
    : QThread(parent)
    , d_ptr(new DFileStatisticsJobPrivate(this))
{

}

DFileStatisticsJob::~DFileStatisticsJob()
{
    stop();
    // ###(zccrs): wait() ?
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

    if (d->state == StoppedState)
        return;

    d->setState(StoppedState);
    d->waitCondition.wakeAll();
}

void DFileStatisticsJob::togglePause()
{
    Q_D(DFileStatisticsJob);

    if (d->state == StoppedState)
        return;

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

    QQueue<DUrl> directory_queue;

    for (const DUrl &url : d->sourceUrlList) {
        d->processFile(url, directory_queue);

        if (!d->stateCheck()) {
            d->setState(StoppedState);

            return;
        }
    }

    while (!directory_queue.isEmpty()) {
        const DUrl &directory_url = directory_queue.dequeue();
        const DDirIteratorPointer &iterator = DFileService::instance()->createDirIterator(this, directory_url, QStringList(), QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);

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
}

DFM_END_NAMESPACE
