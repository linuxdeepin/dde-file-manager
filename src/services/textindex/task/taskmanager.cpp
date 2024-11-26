// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmanager.h"

#include <QMetaType>

SERVICETEXTINDEX_USE_NAMESPACE

namespace {
void registerMetaTypes()
{
    static bool registered = false;
    if (!registered) {
        qRegisterMetaType<IndexTask::Type>();
        qRegisterMetaType<IndexTask::Type>("IndexTask::Type");
        qRegisterMetaType<SERVICETEXTINDEX_NAMESPACE::IndexTask::Type>();
        qRegisterMetaType<SERVICETEXTINDEX_NAMESPACE::IndexTask::Type>("SERVICETEXTINDEX_NAMESPACE::IndexTask::Type");
        registered = true;
        fmDebug() << "Meta types registered successfully";
    }
}
}

TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{
    fmInfo() << "Initializing TaskManager...";
    registerMetaTypes();
    workerThread.start();
    fmInfo() << "TaskManager initialized, worker thread started";
}

TaskManager::~TaskManager()
{
    fmInfo() << "Destroying TaskManager...";
    if (currentTask)
        stopCurrentTask();

    workerThread.quit();
    workerThread.wait();
    fmInfo() << "TaskManager destroyed";
}

bool TaskManager::startTask(IndexTask::Type type, const QString &path)
{
    if (hasRunningTask()) {
        fmWarning() << "Cannot start new task, another task is running";
        return false;
    }

    fmInfo() << "Starting new task for path:" << path;

    // 获取对应的任务处理器
    TaskHandler handler = (type == IndexTask::Type::Create)
            ? TaskHandlers::CreateIndexHandler()
            : TaskHandlers::UpdateIndexHandler();

    currentTask = new IndexTask(type, path, handler);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);

    emit startTaskInThread();
    fmDebug() << "Task started in worker thread";
    return true;
}

void TaskManager::onTaskProgress(IndexTask::Type type, qint64 count)
{
    fmDebug() << "Task progress:" << type << count;
    if (type == IndexTask::Type::Create) {
        emit createIndexCountChanged(count);
    } else {
        emit updateIndexCountChanged(count);
    }
}

void TaskManager::onTaskFinished(IndexTask::Type type, bool success)
{
    if (success) {
        fmInfo() << "Task completed successfully:" << type;
    } else {
        fmWarning() << "Task failed:" << type;
    }

    if (type == IndexTask::Type::Create) {
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
        fmInfo() << "Stopping current task...";
        currentTask->stop();
        cleanupTask();
        fmInfo() << "Task stopped";
    }
}

void TaskManager::cleanupTask()
{
    if (currentTask) {
        fmDebug() << "Cleaning up task resources";
        disconnect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start);
        currentTask->deleteLater();
        currentTask = nullptr;
    }
}
