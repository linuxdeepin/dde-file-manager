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

bool TaskManager::startTask(IndexTask::Type type, const QString &path, bool silent)
{
    Q_ASSERT_X(type == IndexTask::Type::Create || type == IndexTask::Type::Update,
               "Type error", "Only create and update supported");
    if (!IndexUtility::isDefaultIndexedDirectory(path)) {
        fmWarning() << "Cannot start new task, path isn't default directory";
        return false;
    }

    // 如果当前有任务在运行，停止它并将新任务保存为待执行任务
    if (hasRunningTask()) {
        fmInfo() << "Task already running, queuing new task for path:" << path;

        // 停止当前任务
        stopCurrentTask();

        // startTask 的优先级高于 startFileListTask，因此直接重置任务队列
        taskQueue.clear();

        // 将任务加入队列
        TaskQueueItem item;
        item.type = type;
        item.path = path;
        item.silent = silent;
        taskQueue.enqueue(item);

        // 返回true表示任务已经被接受，将在当前任务停止后执行
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "Starting new task for path: " << path << "Type: " << type << "Slient: " << silent;

    // status文件存储了修改时间，清除后外部无法获取时间，外部利用该特性判断索引状态
    if (type == IndexTask::Type::Create) {
        fmInfo() << "Home path task detected, clearing existing index status"
                 << "[Initializing new root indexing task]";
        IndexUtility::removeIndexStatusFile();
    }

    // 获取对应的任务处理器
    TaskHandler handler = getTaskHandler(type);
    if (!handler) {
        fmWarning() << "Unknown task type:" << static_cast<int>(type);
        return false;
    }

    Q_ASSERT(!currentTask);
    currentTask = new IndexTask(type, path, handler);
    currentTask->setSilent(silent);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);
    workerThread.start();

    emit startTaskInThread();
    fmDebug() << "Task started in worker thread";
    return true;
}

bool TaskManager::startFileListTask(IndexTask::Type type, const QStringList &fileList, bool silent)
{
    if (fileList.isEmpty()) {
        fmWarning() << "Cannot start file list task, file list is empty";
        return false;
    }

    // 如果当前有任务在运行，将新任务加入队列
    if (hasRunningTask() || currentTask) {
        fmInfo() << "Task already running, queuing new file list task with" << fileList.size() << "files";

        // 将任务加入队列
        TaskQueueItem item;
        item.type = type;
        item.path = QString("FileList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        item.fileList = fileList;
        item.silent = silent;
        taskQueue.enqueue(item);

        return true;
    }

    // 正常启动任务流程
    fmInfo() << "Starting new file list task with" << fileList.size() << "files. "
             << "Type:" << type << "Slient: " << silent;

    // 获取对应的任务处理器
    TaskHandler handler;
    switch (type) {
    case IndexTask::Type::CreateFileList:
        handler = TaskHandlers::CreateOrUpdateFileListHandler(fileList);
        break;
    case IndexTask::Type::UpdateFileList:
        handler = TaskHandlers::CreateOrUpdateFileListHandler(fileList);
        break;
    case IndexTask::Type::RemoveFileList:
        handler = TaskHandlers::RemoveFileListHandler(fileList);
        break;
    default:
        fmWarning() << "Unknown file list task type:" << static_cast<int>(type);
        return false;
    }

    QString pathId = QString("FileList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));

    Q_ASSERT(!currentTask);
    currentTask = new IndexTask(type, pathId, handler);
    currentTask->setSilent(silent);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);
    workerThread.start();

    emit startTaskInThread();
    fmDebug() << "File list task started in worker thread";
    return true;
}

TaskHandler TaskManager::getTaskHandler(IndexTask::Type type)
{
    switch (type) {
    case IndexTask::Type::Create:
        return TaskHandlers::CreateIndexHandler();
    case IndexTask::Type::Update:
        return TaskHandlers::UpdateIndexHandler();
    default:
        return nullptr;
    }
}

QString TaskManager::typeToString(IndexTask::Type type)
{
    switch (type) {
    case IndexTask::Type::Create:
        return "create";
    case IndexTask::Type::Update:
        return "update";
    case IndexTask::Type::CreateFileList:
        return "create-file-list";
    case IndexTask::Type::UpdateFileList:
        return "update-file-list";
    case IndexTask::Type::RemoveFileList:
        return "remove-file-list";
    default:
        return "unknown";
    }
}

void TaskManager::onTaskProgress(IndexTask::Type type, qint64 count, qint64 total)
{
    if (!currentTask) return;

    fmDebug() << "Task progress:" << type << count;
    emit taskProgressChanged(typeToString(type), currentTask->taskPath(), count, total);
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
    if (IndexUtility::isDefaultIndexedDirectory(taskPath) && !result.success) {
        fmWarning() << "Root indexing failed, clearing status"
                    << "[Root index task failed]";
        IndexUtility::removeIndexStatusFile();
    }

    if (result.success && !result.interrupted) {
        fmInfo() << "Indexing completed successfully, updating status"
                 << "[Index task succeeded]";
        IndexUtility::saveIndexStatus(QDateTime::currentDateTime(), Defines::kIndexVersion);
    }

    emit taskFinished(typeToString(type), taskPath, result.success);
    cleanupTask();

    // 检查是否有待执行的任务
    startNextTask();
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

std::optional<IndexTask::Type> TaskManager::currentTaskType() const
{
    if (!hasRunningTask())
        return std::nullopt;

    return currentTask->taskType();
}

std::optional<QString> TaskManager::currentTaskPath() const
{
    if (!hasRunningTask())
        return std::nullopt;

    return currentTask->taskPath();
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

bool TaskManager::startNextTask()
{
    // 检查队列是否为空
    if (taskQueue.isEmpty()) {
        return false;
    }

    // 从队列中取出下一个任务
    TaskQueueItem nextTask = taskQueue.dequeue();

    fmInfo() << "Number of tasks remaining: " << taskQueue.count();
    fmInfo() << "Starting next queued task of type: " << nextTask.type << "path: " << nextTask.path;

    // 根据任务类型启动相应的任务
    if (nextTask.type == IndexTask::Type::CreateFileList
        || nextTask.type == IndexTask::Type::UpdateFileList
        || nextTask.type == IndexTask::Type::RemoveFileList) {
        // 启动文件列表任务
        return startFileListTask(nextTask.type, nextTask.fileList, nextTask.silent);
    } else {
        // 启动常规任务
        return startTask(nextTask.type, nextTask.path);
    }
}
