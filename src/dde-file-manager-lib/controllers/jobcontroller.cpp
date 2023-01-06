// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jobcontroller.h"
#include "dfileservices.h"

#include <QtConcurrent/QtConcurrent>

#ifndef LOAD_FILE_INTERVAL
#define LOAD_FILE_INTERVAL 50
#endif

JobController::JobController(const DUrl &fileUrl, const DDirIteratorPointer &iterator, bool silent, QObject *parent)
    : QThread(parent)
    , m_silent(silent)
    , m_iterator(iterator)
    , m_fileUrl(fileUrl)
{

}

JobController::JobController(const DUrl &fileUrl, const QStringList &nameFilters,
                             QDir::Filters filters, bool silent, QObject *parent)
    : QThread(parent)
    , m_silent(silent)
    , m_fileUrl(fileUrl)
    , m_nameFilters(nameFilters)
    , m_filters(filters)
{

}

JobController::~JobController()
{
    stop();
    wait();
    if (timer) {
        delete timer;
        timer = nullptr;
    }
}

JobController::State JobController::state() const
{
    return m_state;
}

int JobController::timeCeiling() const
{
    return m_timeCeiling;
}

int JobController::countCeiling() const
{
    return m_countCeiling;
}

void JobController::start()
{
    if (m_state == Started) {
        return;
    }

    if (m_state == Paused) {
        setState(Started);
        waitCondition.wakeAll();

        return;
    }
    setState(Started);
    QThread::start(TimeCriticalPriority);
}

void JobController::pause()
{
    if (m_state != Started)
        return;

    setState(Paused);
}

void JobController::stop()
{
    if (m_state == Stoped)
        return;

    if (m_iterator)
        m_iterator->close();

    setState(Stoped);

    waitCondition.wakeAll();
}

void JobController::stopAndDeleteLater()
{
    stop();

    if (!isRunning()) {
        deleteLater();
    } else {
        disconnect(this, &JobController::finished, this, &JobController::deleteLater);
        connect(this, &JobController::finished, this, &JobController::deleteLater);
    }
}

void JobController::setTimeCeiling(int timeCeiling)
{
    if (isRunning())
        return;

    m_timeCeiling = timeCeiling;
}

void JobController::setCountCeiling(int countCeiling)
{
    if (isRunning())
        return;

    m_countCeiling = countCeiling;
}

void JobController::run()
{
    m_updateFinished = false;
    if (!m_iterator) {
        const auto &&list = DFileService::instance()->getChildren(this, m_fileUrl, m_nameFilters, m_filters, QDirIterator::NoIteratorFlags, m_silent);

        m_updateFinished = true;
        emit childrenUpdated(list);
        emit addChildrenList(list);

        setState(Stoped);

        return;
    }

    QQueue<DAbstractFileInfoPointer> fileInfoQueue;

    if (!timer)
        timer = new QElapsedTimer();

    timer->restart();

    bool update_children = true;

    const DAbstractFileInfoPointer &rootInfo = DFileService::instance()->createFileInfo(this, m_fileUrl);
    if (rootInfo && !rootInfo->hasOrderly() && fileInfoQueue.count() > 0) {
        update_children = false;
        emit childrenUpdated(fileInfoQueue);
        emit addChildrenList(fileInfoQueue);
    }
    while (m_iterator->hasNext()) {
        if (m_state == Paused) {
            mutex.lock();
            waitCondition.wait(&mutex);
            mutex.unlock();
        }

        if (m_state == Stoped) {
            break;
        }
        if (!m_iterator->next().isValid())
            continue;

        DAbstractFileInfoPointer fileinfo;
        fileinfo = m_iterator->fileInfo();
        /*fix bug 49039 解决多次调用m_iterator->hasNext导致有重复结果，这里去重*/
        if (!fileinfo || fileInfoQueue.contains(fileinfo))
            continue;

        if (update_children) {
            fileInfoQueue.enqueue(fileinfo);

            if (timer->elapsed() > m_timeCeiling || fileInfoQueue.count() > m_countCeiling) {
                update_children = false;
                emit childrenUpdated(fileInfoQueue);
                emit addChildrenList(fileInfoQueue);

                fileInfoQueue.clear();
            }
        } else {
            fileInfoQueue.enqueue(fileinfo);

            if (timer->elapsed() > m_timeCeiling || fileInfoQueue.count() > m_countCeiling) {
                timer->restart();

                emit addChildrenList(fileInfoQueue);

                fileInfoQueue.clear();
            }

            emit addChildren(fileinfo);

        }
    }
    //刷新已完成
    m_updateFinished = true;

    if (m_state != Stoped)
        emit addChildren(DAbstractFileInfoPointer(nullptr), true);

    if (timer) {
        delete timer;
        timer = Q_NULLPTR;
    }
    if (update_children) {
        emit childrenUpdated(fileInfoQueue);
        emit addChildrenList(fileInfoQueue);
    }

    setState(Stoped);
}

void JobController::setState(JobController::State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit stateChanged(state);
}
