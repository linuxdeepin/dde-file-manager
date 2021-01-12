/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "jobcontroller.h"
#include "dfileservices.h"
#ifdef  FULLTEXTSEARCH_ENABLE
#include "fulltextsearch.h"
#endif

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
    if (timer)
        delete timer;
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
    if (!isRunning()) {
        deleteLater();
    } else {
        disconnect(this, &JobController::finished, this, &JobController::deleteLater);
        connect(this, &JobController::finished, this, &JobController::deleteLater);
    }

    stop();
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
        if (fileinfo) {
            /*fix bug 49039 解决多次调用m_iterator->hasNext导致有重复结果，这里去重*/
            if (fileInfoQueue.contains(fileinfo)) {
                continue;
            }
        }
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
#ifdef  FULLTEXTSEARCH_ENABLE
    DFMFullTextSearchManager::getInstance()->setSearchState(state);
#endif
    emit stateChanged(state);
}
