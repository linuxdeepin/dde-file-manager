// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
        fmDebug() << "[TaskManager] Meta types registered successfully";
    }
}

}   // namespace

TaskManager::TaskManager(QObject *parent)
    : QObject(parent)
{
    fmInfo() << "[TaskManager] Initializing TaskManager instance";
    registerMetaTypes();
    fmInfo() << "[TaskManager] TaskManager initialization completed";
}

TaskManager::~TaskManager()
{
    fmInfo() << "[TaskManager] Destroying TaskManager instance";
    if (currentTask) {
        fmInfo() << "[TaskManager] Stopping current task before destruction";
        stopCurrentTask();
    }

    if (workerThread.isRunning()) {
        fmInfo() << "[TaskManager] Stopping worker thread";
        workerThread.quit();
        if (!workerThread.wait(5000)) {   // 等待5秒
            fmWarning() << "[TaskManager] Worker thread did not stop within timeout, forcing termination";
            workerThread.terminate();
            workerThread.wait(1000);
        }
    }
    fmInfo() << "[TaskManager] TaskManager destroyed successfully";
}

// 单路径版本，调用多路径版本保持兼容性
bool TaskManager::startTask(IndexTask::Type type, const QString &path, bool silent)
{
    fmDebug() << "[TaskManager::startTask] Single path task request - type:" << static_cast<int>(type)
              << "path:" << path << "silent:" << silent;
    return startTask(type, QStringList { path }, silent);
}

// 多路径版本的startTask实现
bool TaskManager::startTask(IndexTask::Type type, const QStringList &pathList, bool silent)
{
    Q_ASSERT_X(type == IndexTask::Type::Create || type == IndexTask::Type::Update,
               "Type error", "Only create and update supported");

    fmInfo() << "[TaskManager::startTask] Multi-path task request - type:" << static_cast<int>(type)
             << "paths:" << pathList.size() << "silent:" << silent;

    // 检查路径列表是否为空
    if (pathList.isEmpty()) {
        fmWarning() << "[TaskManager::startTask] Cannot start task - path list is empty";
        return false;
    }

    // 所有路径都必须是默认索引目录
    bool allPathsValid = true;
    QStringList invalidPaths;
    for (const auto &path : pathList) {
        if (!IndexUtility::isDefaultIndexedDirectory(path)) {
            fmWarning() << "[TaskManager::startTask] Invalid path detected:" << path;
            invalidPaths.append(path);
            allPathsValid = false;
        }
    }

    if (!allPathsValid) {
        fmWarning() << "[TaskManager::startTask] Cannot start task - invalid paths found:" << invalidPaths;
        return false;
    }

    // 获取第一个路径作为任务的主路径（用于日志和进度通知）
    QString primaryPath = pathList.first();

    // 如果当前有任务在运行，停止它并将新任务保存为待执行任务
    if (hasRunningTask()) {
        fmInfo() << "[TaskManager::startTask] Current task running, queuing new task - paths:" << pathList.size()
                 << "primary:" << primaryPath;

        // 停止当前任务
        stopCurrentTask();

        // startTask 的优先级高于 startFileListTask，因此直接重置任务队列
        if (!taskQueue.isEmpty()) {
            fmInfo() << "[TaskManager::startTask] Clearing existing task queue with" << taskQueue.size() << "pending tasks";
            taskQueue.clear();
        }

        // 将任务加入队列
        TaskQueueItem item;
        item.type = type;
        item.path = primaryPath;   // 保留主路径用于兼容现有代码
        item.pathList = pathList;   // 保存所有路径
        item.silent = silent;
        taskQueue.enqueue(item);

        fmInfo() << "[TaskManager::startTask] Task queued successfully, will execute after current task stops";
        // 返回true表示任务已经被接受，将在当前任务停止后执行
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "[TaskManager::startTask] Starting new task immediately - paths:" << pathList.size()
             << "primary:" << primaryPath << "type:" << static_cast<int>(type) << "silent:" << silent;

    // status文件存储了修改时间，清除后外部无法获取时间，外部利用该特性判断索引状态
    if (type == IndexTask::Type::Create) {
        fmInfo() << "[TaskManager::startTask] Create task detected, clearing existing index status";
        IndexUtility::removeIndexStatusFile();
        // 创建索引的任务开销巨大，避免任务未完成时进程退出后，重复进入创建任务
        IndexUtility::saveIndexStatus(QDateTime::currentDateTime());
    }

    // 获取对应的任务处理器
    TaskHandler handler = getTaskHandler(type);
    if (!handler) {
        fmCritical() << "[TaskManager::startTask] Unknown task type:" << static_cast<int>(type);
        return false;
    }

    Q_ASSERT(!currentTask);
    // 创建新的任务对象，使用路径列表作为输入
    // 注意：为了最小修改现有代码，我们仍然将主路径作为任务路径，但在handler中会使用整个路径列表
    currentTask = new IndexTask(type, primaryPath, [handler, pathList](const QString &, TaskState &state) -> HandlerResult {
        fmDebug() << "[TaskManager::startTask] Executing task handler for" << pathList.size() << "paths";
        // 在这个lambda中，我们会对每个路径执行原始的handler
        HandlerResult finalResult { true, false, false, false };

        for (const auto &path : pathList) {
            if (!state.isRunning()) {
                fmInfo() << "[TaskManager::startTask] Task execution interrupted during path processing";
                finalResult.interrupted = true;
                break;
            }

            fmDebug() << "[TaskManager::startTask] Processing path:" << path;
            // 对每个路径执行handler
            HandlerResult pathResult = handler(path, state);

            // 如果任何一个路径处理失败，整个任务就失败
            if (!pathResult.success) {
                fmWarning() << "[TaskManager::startTask] Path processing failed:" << path;
                finalResult.success = false;
            }

            if (pathResult.fatal) {
                fmCritical() << "[TaskManager::startTask] Fatal error occurred during path processing:" << path;
                finalResult.fatal = true;
                break;
            }

            // 如果被中断，设置中断标志并退出循环
            if (pathResult.interrupted) {
                fmInfo() << "[TaskManager::startTask] Path processing interrupted:" << path;
                finalResult.interrupted = true;
                break;
            }

            if (pathResult.useAnything) {
                fmInfo() << "[TaskManager::startTask] Using ANYTHING for file discovery, skipping remaining paths";
                break;
            }
        }

        fmInfo() << "[TaskManager::startTask] Task handler execution completed - success:" << finalResult.success
                 << "interrupted:" << finalResult.interrupted << "fatal:" << finalResult.fatal;
        return finalResult;
    });

    currentTask->setSilent(silent);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);
    workerThread.start();

    // Mark index state as dirty before starting task
    IndexUtility::setIndexState(IndexUtility::IndexState::Dirty);

    emit startTaskInThread();
    fmInfo() << "[TaskManager::startTask] Task started successfully in worker thread";
    return true;
}

bool TaskManager::startFileListTask(IndexTask::Type type, const QStringList &fileList, bool silent)
{
    fmInfo() << "[TaskManager::startFileListTask] File list task request - type:" << static_cast<int>(type)
             << "files:" << fileList.size() << "silent:" << silent;

    if (fileList.isEmpty()) {
        fmWarning() << "[TaskManager::startFileListTask] Cannot start task - file list is empty";
        return false;
    }

    // 如果当前有任务在运行，将新任务加入队列
    if (hasRunningTask() || currentTask) {
        fmInfo() << "[TaskManager::startFileListTask] Current task running, queuing file list task with"
                 << fileList.size() << "files";

        // 将任务加入队列
        TaskQueueItem item;
        item.type = type;
        item.path = QString("FileList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        item.fileList = fileList;
        item.silent = silent;
        taskQueue.enqueue(item);

        fmDebug() << "[TaskManager::startFileListTask] File list task queued successfully";
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "[TaskManager::startFileListTask] Starting file list task immediately - files:" << fileList.size()
             << "type:" << static_cast<int>(type) << "silent:" << silent;

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
        fmCritical() << "[TaskManager::startFileListTask] Unknown file list task type:" << static_cast<int>(type);
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

    // Mark index state as dirty before starting task
    IndexUtility::setIndexState(IndexUtility::IndexState::Dirty);

    emit startTaskInThread();
    fmDebug() << "[TaskManager::startFileListTask] File list task started successfully in worker thread";
    return true;
}

bool TaskManager::startFileMoveTask(const QHash<QString, QString> &movedFiles, bool silent)
{
    fmInfo() << "[TaskManager::startFileMoveTask] File move task request - moves:" << movedFiles.size()
             << "silent:" << silent;

    if (movedFiles.isEmpty()) {
        fmWarning() << "[TaskManager::startFileMoveTask] Cannot start task - moved files list is empty";
        return false;
    }

    // 如果当前有任务在运行，将新任务加入队列
    if (hasRunningTask() || currentTask) {
        fmInfo() << "[TaskManager::startFileMoveTask] Current task running, queuing file move task with"
                 << movedFiles.size() << "moves";

        // 将任务加入队列
        TaskQueueItem item;
        item.type = IndexTask::Type::MoveFileList;
        item.path = QString("MoveList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        item.movedFiles = movedFiles;
        item.silent = silent;
        taskQueue.enqueue(item);

        fmDebug() << "[TaskManager::startFileMoveTask] File move task queued successfully";
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "[TaskManager::startFileMoveTask] Starting file move task immediately - moves:" << movedFiles.size()
             << "silent:" << silent;

    // 获取对应的任务处理器
    TaskHandler handler = TaskHandlers::MoveFileListHandler(movedFiles);
    if (!handler) {
        fmCritical() << "[TaskManager::startFileMoveTask] Failed to create move file list handler";
        return false;
    }

    QString pathId = QString("MoveList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));

    Q_ASSERT(!currentTask);
    currentTask = new IndexTask(IndexTask::Type::MoveFileList, pathId, handler);
    currentTask->setSilent(silent);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);
    workerThread.start();

    // Mark index state as dirty before starting task
    IndexUtility::setIndexState(IndexUtility::IndexState::Dirty);

    emit startTaskInThread();
    fmDebug() << "[TaskManager::startFileMoveTask] File move task started successfully in worker thread";
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
        fmWarning() << "[TaskManager::getTaskHandler] Unknown task type:" << static_cast<int>(type);
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
    case IndexTask::Type::MoveFileList:
        return "move-file-list";
    default:
        fmWarning() << "[TaskManager::typeToString] Unknown task type:" << static_cast<int>(type);
        return "unknown";
    }
}

void TaskManager::onTaskProgress(IndexTask::Type type, qint64 count, qint64 total)
{
    if (!currentTask) {
        fmWarning() << "[TaskManager::onTaskProgress] Received progress update but no current task exists";
        return;
    }

    emit taskProgressChanged(typeToString(type), currentTask->taskPath(), count, total);
}

void TaskManager::onTaskFinished(IndexTask::Type type, HandlerResult result)
{
    if (!currentTask) {
        fmWarning() << "[TaskManager::onTaskFinished] Received task finished signal but no current task exists";
        return;
    }

    QString taskPath = currentTask->taskPath();
    fmInfo() << "[TaskManager::onTaskFinished] Task finished - type:" << static_cast<int>(type)
             << "path:" << taskPath << "success:" << result.success << "interrupted:" << result.interrupted;

    if (!result.success && type == IndexTask::Type::Update) {
        // 检查是否是由于索引损坏导致的失败
        if (currentTask->isIndexCorrupted()) {
            fmWarning() << "[TaskManager::onTaskFinished] Update task failed due to index corruption, attempting rebuild - path:" << taskPath;

            // 清理损坏的索引
            IndexUtility::clearIndexDirectory();

            // 启动新的创建任务
            cleanupTask();   // 清理当前失败的任务

            // 原始代码可能是以单一路径处理的，这里修改为支持多路径
            // 如果原始任务是多路径任务，我们需要获取对应的所有路径
            if (!taskQueue.isEmpty() && taskQueue.head().pathList.contains(taskPath)) {
                // 队列中有包含这个路径的任务，直接让队列处理
                fmInfo() << "[TaskManager::onTaskFinished] Found queued task containing corrupted path, letting queue handle rebuild";
            } else {
                // 单路径情况，直接创建新任务
                fmInfo() << "[TaskManager::onTaskFinished] Starting rebuild task for corrupted index - path:" << taskPath;
                if (startTask(IndexTask::Type::Create, taskPath)) {
                    return;   // 新任务已启动，等待其完成
                } else {
                    fmCritical() << "[TaskManager::onTaskFinished] Failed to start rebuild task for path:" << taskPath;
                }
            }
        } else {
            fmInfo() << "[TaskManager::onTaskFinished] Update task failed but index is not corrupted, skipping rebuild - path:" << taskPath;
        }
    }

    fmDebug() << "[TaskManager::onTaskFinished] Task" << typeToString(type) << "for path" << taskPath
              << (result.success ? "completed successfully" : "failed");

    // 如果是根目录的任务，更新状态文件
    if (IndexUtility::isDefaultIndexedDirectory(taskPath) && !result.success) {
        fmWarning() << "[TaskManager::onTaskFinished] Root indexing failed, clearing status - path:" << taskPath;
        IndexUtility::removeIndexStatusFile();
    }

    if (result.success) {
        if (!result.interrupted || type == IndexTask::Type::Create) {
            fmDebug() << "[TaskManager::onTaskFinished] Task completed successfully, updating index status";
            IndexUtility::saveIndexStatus(QDateTime::currentDateTime());
        }
    }
    emit taskFinished(typeToString(type), taskPath, result.success);
    cleanupTask();

    // 检查是否有待执行的任务
    if (startNextTask()) {
        fmInfo() << "[TaskManager::onTaskFinished] Started next queued task";
    } else {
        fmDebug() << "[TaskManager::onTaskFinished] No more tasks in queue";
        // Mark index state as clean only when all tasks are completed successfully
        if (result.success && !result.interrupted) {
            IndexUtility::setIndexState(IndexUtility::IndexState::Clean);
            fmInfo() << "[TaskManager::onTaskFinished] All tasks completed, index state set to clean";
        }
    }
}

bool TaskManager::hasRunningTask() const
{
    return currentTask && currentTask->isRunning();
}

bool TaskManager::hasQueuedTasks() const
{
    return !taskQueue.isEmpty();
}

void TaskManager::stopCurrentTask()
{
    if (currentTask) {
        fmInfo() << "[TaskManager::stopCurrentTask] Stopping current task - type:" << static_cast<int>(currentTask->taskType())
                 << "path:" << currentTask->taskPath();
        currentTask->stop();
    } else {
        fmDebug() << "[TaskManager::stopCurrentTask] No current task to stop";
    }
}

std::optional<IndexTask::Type> TaskManager::currentTaskType() const
{
    if (!hasRunningTask()) {
        return std::nullopt;
    }

    return currentTask->taskType();
}

std::optional<QString> TaskManager::currentTaskPath() const
{
    if (!hasRunningTask()) {
        return std::nullopt;
    }

    return currentTask->taskPath();
}

void TaskManager::cleanupTask()
{
    if (currentTask) {
        fmDebug() << "[TaskManager::cleanupTask] Cleaning up task resources - type:" << static_cast<int>(currentTask->taskType())
                  << "path:" << currentTask->taskPath();
        disconnect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start);
        currentTask->deleteLater();
        currentTask = nullptr;
        fmDebug() << "[TaskManager::cleanupTask] Task cleanup completed";
    }
}

bool TaskManager::startNextTask()
{
    // 检查队列是否为空
    if (taskQueue.isEmpty()) {
        fmDebug() << "[TaskManager::startNextTask] No tasks in queue";
        return false;
    }

    // 从队列中取出下一个任务
    TaskQueueItem nextTask = taskQueue.dequeue();

    fmInfo() << "[TaskManager::startNextTask] Starting next queued task - type:" << static_cast<int>(nextTask.type)
             << "remaining in queue:" << taskQueue.count();

    // 根据任务类型启动相应的任务
    if (nextTask.type == IndexTask::Type::CreateFileList
        || nextTask.type == IndexTask::Type::UpdateFileList
        || nextTask.type == IndexTask::Type::RemoveFileList) {
        // 启动文件列表任务
        fmInfo() << "[TaskManager::startNextTask] Starting queued file list task - type:" << static_cast<int>(nextTask.type)
                 << "files:" << nextTask.fileList.size();
        return startFileListTask(nextTask.type, nextTask.fileList, nextTask.silent);
    } else if (nextTask.type == IndexTask::Type::MoveFileList) {
        // 启动文件移动任务
        fmInfo() << "[TaskManager::startNextTask] Starting queued file move task - moves:" << nextTask.movedFiles.size();
        return startFileMoveTask(nextTask.movedFiles, nextTask.silent);
    } else if (!nextTask.pathList.isEmpty()) {
        // 启动多路径任务
        fmInfo() << "[TaskManager::startNextTask] Starting queued multi-path task - type:" << static_cast<int>(nextTask.type)
                 << "paths:" << nextTask.pathList.size();
        return startTask(nextTask.type, nextTask.pathList, nextTask.silent);
    } else {
        // 启动单路径任务
        fmInfo() << "[TaskManager::startNextTask] Starting queued single-path task - type:" << static_cast<int>(nextTask.type)
                 << "path:" << nextTask.path;
        return startTask(nextTask.type, nextTask.path, nextTask.silent);
    }
}
