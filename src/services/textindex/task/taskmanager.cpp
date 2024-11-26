#include "taskmanager.h"

SERVICETEXTINDEX_USE_NAMESPACE

TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{
    workerThread.start();
}

TaskManager::~TaskManager()
{
    if (currentTask)
        stopCurrentTask();

    workerThread.quit();
    workerThread.wait();
}

bool TaskManager::startTask(IndexTask::Type type, const QString &path)
{
    if (hasRunningTask())
        return false;

    currentTask = new IndexTask(type, path);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished);

    currentTask->start();
    return true;
}

void TaskManager::onTaskProgress(IndexTask::Type type, int count)
{
    if (type == IndexTask::Create) {
        emit createIndexCountChanged(count);
    } else {
        emit updateIndexCountChanged(count);
    }
}

void TaskManager::onTaskFinished(IndexTask::Type type, bool success)
{
    if (type == IndexTask::Create) {
        if (success)
            emit createSuccessful();
        else
            emit createFailed();
    } else {
        if (success)
            emit updateSuccessful();
        else
            emit updateFailed();
    }
    cleanupTask();
}

bool TaskManager::hasRunningTask() const
{
    return currentTask && currentTask->isRunning();
}

void TaskManager::stopCurrentTask()
{
    if (currentTask) {
        currentTask->stop();
        cleanupTask();
    }
}

void TaskManager::cleanupTask()
{
    if (currentTask) {
        currentTask->deleteLater();
        currentTask = nullptr;
    }
}
