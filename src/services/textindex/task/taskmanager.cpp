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

// 单路径版本，调用多路径版本保持兼容性
bool TaskManager::startTask(IndexTask::Type type, const QString &path, bool silent)
{
    return startTask(type, QStringList { path }, silent);
}

// 多路径版本的startTask实现
bool TaskManager::startTask(IndexTask::Type type, const QStringList &pathList, bool silent)
{
    Q_ASSERT_X(type == IndexTask::Type::Create || type == IndexTask::Type::Update,
               "Type error", "Only create and update supported");

    // 检查路径列表是否为空
    if (pathList.isEmpty()) {
        fmWarning() << "Cannot start new task, path list is empty";
        return false;
    }

    // 所有路径都必须是默认索引目录
    bool allPathsValid = true;
    for (const auto &path : pathList) {
        if (!IndexUtility::isDefaultIndexedDirectory(path)) {
            fmWarning() << "Cannot start new task, path isn't default directory:" << path;
            allPathsValid = false;
            break;
        }
    }

    if (!allPathsValid) {
        return false;
    }

    // 获取第一个路径作为任务的主路径（用于日志和进度通知）
    QString primaryPath = pathList.first();

    // 如果当前有任务在运行，停止它并将新任务保存为待执行任务
    if (hasRunningTask()) {
        fmInfo() << "Task already running, queuing new task for" << pathList.size() << "paths, primary:" << primaryPath;

        // 停止当前任务
        stopCurrentTask();

        // startTask 的优先级高于 startFileListTask，因此直接重置任务队列
        taskQueue.clear();

        // 将任务加入队列
        TaskQueueItem item;
        item.type = type;
        item.path = primaryPath;   // 保留主路径用于兼容现有代码
        item.pathList = pathList;   // 保存所有路径
        item.silent = silent;
        taskQueue.enqueue(item);

        // 返回true表示任务已经被接受，将在当前任务停止后执行
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "Starting new task for" << pathList.size() << "paths, primary:" << primaryPath
             << "Type:" << type << "Silent:" << silent;

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
    // 创建新的任务对象，使用路径列表作为输入
    // 注意：为了最小修改现有代码，我们仍然将主路径作为任务路径，但在handler中会使用整个路径列表
    currentTask = new IndexTask(type, primaryPath, [handler, pathList](const QString &, TaskState &state) -> HandlerResult {
        // 在这个lambda中，我们会对每个路径执行原始的handler
        HandlerResult finalResult { true, false };

        for (const auto &path : pathList) {
            if (!state.isRunning()) {
                finalResult.interrupted = true;
                break;
            }

            // 对每个路径执行handler
            HandlerResult pathResult = handler(path, state);

            // 如果任何一个路径处理失败，整个任务就失败
            if (!pathResult.success) {
                finalResult.success = false;
            }

            // 如果被中断，设置中断标志并退出循环
            if (pathResult.interrupted) {
                finalResult.interrupted = true;
                break;
            }

            if (pathResult.useAnything) {
                fmInfo() << "Since anything gets all directory paths, it skips the other path tasks.";
                break;
            }
        }

        return finalResult;
    });

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

            // 原始代码可能是以单一路径处理的，这里修改为支持多路径
            // 如果原始任务是多路径任务，我们需要获取对应的所有路径
            if (!taskQueue.isEmpty() && taskQueue.head().pathList.contains(taskPath)) {
                // 队列中有包含这个路径的任务，直接让队列处理
                fmInfo() << "Found queued task containing the corrupted path, letting queue handle it";
            } else {
                // 单路径情况，直接创建新任务
                if (startTask(IndexTask::Type::Create, taskPath)) {
                    return;   // 新任务已启动，等待其完成
                }
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

    // 根据任务类型启动相应的任务
    if (nextTask.type == IndexTask::Type::CreateFileList
        || nextTask.type == IndexTask::Type::UpdateFileList
        || nextTask.type == IndexTask::Type::RemoveFileList) {
        // 启动文件列表任务
        fmInfo() << "Starting next queued file list task of type: " << nextTask.type;
        return startFileListTask(nextTask.type, nextTask.fileList, nextTask.silent);
    } else if (!nextTask.pathList.isEmpty()) {
        // 启动多路径任务
        fmInfo() << "Starting next queued multi-path task of type: " << nextTask.type << "with" << nextTask.pathList.size() << "paths";
        return startTask(nextTask.type, nextTask.pathList, nextTask.silent);
    } else {
        // 启动单路径任务
        fmInfo() << "Starting next queued task of type: " << nextTask.type << "path: " << nextTask.path;
        return startTask(nextTask.type, nextTask.path, nextTask.silent);
    }
}
