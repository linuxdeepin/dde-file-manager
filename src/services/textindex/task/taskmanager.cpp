// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmanager.h"
#include "utils/indexutility.h"

#include <QMetaType>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDateTime>

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
        qRegisterMetaType<HandlerResult>();
        registered = true;
        fmDebug() << "Meta types registered successfully";
    }
}

}   // namespace

TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{
    fmInfo() << "Initializing TaskManager...";
    registerMetaTypes();
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
    if (!IndexUtility::isDefaultIndexedDirectory(path)) {
        fmWarning() << "Cannot start new task, path isn't default direcroty";
        return false;
    }

    // 如果当前有任务在运行，停止它并将新任务保存为待执行任务
    if (hasRunningTask() || currentTask) {
        fmInfo() << "Task already running, stopping current task and queuing new task for path:" << path;
        
        // 保存待执行的任务信息
        pendingTaskType = type;
        pendingTaskPath = path;
        hasPendingTask = true;
        
        // 停止当前任务
        stopCurrentTask();
        
        // 返回true表示任务已经被接受，将在当前任务停止后执行
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "Starting new task for path:" << path;

    // status文件存储了修改时间，清除后外部无法获取时间，外部利用该特性判断索引状态
    if (type == IndexTask::Type::Create) {
        fmInfo() << "Home path task detected, clearing existing index status"
                 << "[Initializing new root indexing task]";
        IndexUtility::removeIndexStatusFile();
    }

    // 获取对应的任务处理器
    TaskHandler handler;
    switch (type) {
    case IndexTask::Type::Create:
        handler = TaskHandlers::CreateIndexHandler();
        break;
    case IndexTask::Type::Update:
        handler = TaskHandlers::UpdateIndexHandler();
        break;
    default:
        fmWarning() << "Unknown task type:" << static_cast<int>(type);
        return false;
    }

    Q_ASSERT(!currentTask);
    currentTask = new IndexTask(type, path, handler);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);
    workerThread.start();

    emit startTaskInThread();
    fmDebug() << "Task started in worker thread";
    return true;
}

QString TaskManager::typeToString(IndexTask::Type type)
{
    switch (type) {
    case IndexTask::Type::Create:
        return "create";
    case IndexTask::Type::Update:
        return "update";
    case IndexTask::Type::Remove:
        return "remove";
    default:
        return "unknown";
    }
}

void TaskManager::onTaskProgress(IndexTask::Type type, qint64 count)
{
    if (!currentTask) return;

    fmDebug() << "Task progress:" << type << count;
    emit taskProgressChanged(typeToString(type), currentTask->taskPath(), count);
}

void TaskManager::onTaskFinished(IndexTask::Type type, HandlerResult result)
{
    if (!currentTask) return;

    QString taskPath = currentTask->taskPath();

    if (!result.success && type == IndexTask::Type::Update) {
        // 检查是否是由于索引损坏导致的失败
        if (currentTask->isIndexCorrupted()) {
            fmWarning() << "Update task failed due to index corruption for path:" << taskPath << ", trying to rebuild index";

            // 清理损坏的索引
            IndexUtility::clearIndexDirectory();

            // 启动新的创建任务
            cleanupTask();   // 清理当前失败的任务
            if (startTask(IndexTask::Type::Create, taskPath)) {
                return;   // 新任务已启动，等待其完成
            }
        } else {
            fmInfo() << "Update task failed but index is not corrupted, skipping rebuild for path:" << taskPath;
        }
    }

    fmInfo() << "Task" << typeToString(type) << "for path" << taskPath
             << (result.success ? "completed successfully" : "failed");

    // 如果是根目录的任务，更新状态文件
    if (IndexUtility::isDefaultIndexedDirectory(taskPath)) {
        if (!result.success) {
            fmWarning() << "Root indexing failed, clearing status"
                        << "[Root index task failed]";
            IndexUtility::removeIndexStatusFile();
        } else if (result.success && !result.interrupted) {
            fmInfo() << "Root indexing completed successfully, updating status"
                     << "[Root index task succeeded]";
            IndexUtility::saveIndexStatus(QDateTime::currentDateTime(), Defines::kIndexVersion);
        }
    }

    emit taskFinished(typeToString(type), taskPath, result.success);
    cleanupTask();
    
    // 检查是否有待执行的任务
    startPendingTaskIfAny();
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
    }
}

void TaskManager::cleanupTask()
{
    if (currentTask) {
        fmInfo() << "Cleaning up task resources";
        disconnect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start);
        currentTask->deleteLater();
        currentTask = nullptr;
    }
}

void TaskManager::startPendingTaskIfAny()
{
    // 检查是否有待执行的任务
    if (hasPendingTask) {
        fmInfo() << "Starting pending task for path:" << pendingTaskPath;
        
        // 保存任务信息并清除待执行标记
        IndexTask::Type type = pendingTaskType;
        QString path = pendingTaskPath;
        hasPendingTask = false;
                
        // 启动待执行的任务
        startTask(type, path);
    }
}
