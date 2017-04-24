#include "jobcontroller.h"
#include "dfileservices.h"

#include <QtConcurrent/QtConcurrent>

#ifndef LOAD_FILE_INTERVAL
#define LOAD_FILE_INTERVAL 50
#endif

JobController::JobController(const DUrl &fileUrl, const DDirIteratorPointer &iterator, QObject *parent)
    : QThread(parent)
    , m_iterator(iterator)
    , m_fileUrl(fileUrl)
{

}

JobController::JobController(const DUrl &fileUrl, const QStringList &nameFilters,
                             QDir::Filters filters, QObject *parent)
    : QThread(parent)
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

void JobController::run()
{
    if (!m_iterator) {
        emit childrenUpdated(DFileService::instance()->getChildren(this, m_fileUrl, m_nameFilters, m_filters));

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

        m_iterator->next();

        if (update_children) {
            fileInfoQueue.enqueue(m_iterator->fileInfo());

            if (timer->elapsed() > 5000 || fileInfoQueue.count() > 10000) {
                update_children = false;

                emit childrenUpdated(fileInfoQueue);

                fileInfoQueue.clear();

                delete timer;
                timer = Q_NULLPTR;
            }
        } else {
            emit addChildren(m_iterator->fileInfo());

            QThread::msleep(LOAD_FILE_INTERVAL);
        }
    }


    if (update_children) {
        emit childrenUpdated(fileInfoQueue);
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
