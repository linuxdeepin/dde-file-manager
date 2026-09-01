// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmanager.h"
#include "taskqueueutils.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"
#include "env/envdetector.h"

#include <QMetaType>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>

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

TaskQueueItem createCompensationTaskItem(const QStringList &paths)
{
    TaskQueueItem item;
    item.type = IndexTask::Type::UpdateFileList;
    item.grade = IndexTask::Grade::Light;
    item.path = paths.isEmpty() ? QString() : paths.first();
    item.fileList = paths;
    return item;
}

}   // namespace

TaskManager::TaskManager(const IndexContext *context, QObject *parent)
    : QObject(parent),
      m_context(context)
{
    fmInfo() << "[TaskManager] Initializing TaskManager instance";
    registerMetaTypes();

    connect(&EnvDetector::instance(), &EnvDetector::envStateChanged,
            this, &TaskManager::onEnvStateChanged);

    fmInfo() << "[TaskManager] TaskManager initialization completed";
}

TaskManager::~TaskManager()
{
    fmInfo() << "[TaskManager] Destroying TaskManager instance";
    if (currentTask) {
        fmInfo() << "[TaskManager] Stopping current task before destruction";
        stopCurrentTask();
        currentTask->disconnect();
    }

    if (workerThread.isRunning()) {
        fmInfo() << "[TaskManager] Stopping worker thread";
        workerThread.quit();
        // worker 为事件循环模型(QThread::exec)，quit() 属协作式退出：当前正在处理的槽
        // (IndexWriter::commit/optimize 或文件内容提取)执行完后事件循环才会返回。Lucene++ 的长
        // 操作没有中途取消接口，进入后只能等待其执行完成。
        // 严禁使用 terminate() —— 其底层是 pthread_cancel，会向 worker 线程注入 forced-unwind
        // 异常；若 worker 此刻正处在带 catch(...) 的调用链中(如 taskhandler 的索引处理、docparser
        // 的内容提取)，异常会被 catch(...) 截留且未 rethrow，从而触发 "FATAL: exception not
        // rethrown"，导致整个进程被 SIGABRT 中止(参见历史 dde-file-manager coredump)。
        // 此处给足等待时间，超时则交由进程退出兜底(进程 _exit 时线程由内核直接终止，不走 unwind)。
        if (!workerThread.wait(5000)) {
            fmWarning() << "[TaskManager] Worker thread still running after 5s, "
                           "will be cleaned up on process exit (terminate() removed to avoid SIGABRT)";
            _exit(1);
        }
    }
    fmInfo() << "[TaskManager] TaskManager destroyed successfully";
}

// 单路径版本，调用多路径版本保持兼容性
bool TaskManager::startTask(IndexTask::Type type, const QString &path)
{
    fmDebug() << "[TaskManager::startTask] Single path task request - type:" << static_cast<int>(type)
              << "path:" << path;
    return startTask(type, QStringList { path });
}

// 多路径版本的startTask实现
bool TaskManager::startTask(IndexTask::Type type, const QStringList &pathList,
                            IndexTask::Grade grade, bool forceBypass)
{
    Q_ASSERT_X(type == IndexTask::Type::Create || type == IndexTask::Type::Update,
               "Type error", "Only create and update supported");

    fmInfo() << "[TaskManager::startTask] Multi-path task request - type:" << static_cast<int>(type)
             << "paths:" << pathList.size()
             << "grade:" << static_cast<int>(grade) << "forceBypass:" << forceBypass;

    // 如果 grade 未指定，自动判定
    if (grade == IndexTask::Grade::None) {
        grade = (type == IndexTask::Type::Create) ? IndexTask::Grade::Heavy : gradeUpdateTask();
    }

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

    // Create 任务的 createInProgress 标记必须在入队检查之前完成，否则当任务被环境
    // 阻塞入队时，服务重启后队列丢失，恢复逻辑会误判为 Light 级 Update。
    //
    // 注意：此处不调用 saveIndexStatus() 写入版本号。版本号仅在 CreateIndexHandler
    // 实际执行并创建 .old 目录后才写入（taskhandler.cpp），或任务成功完成后由
    // updateIndexStatusOnSuccess 写入。若在入队前就写入新版本号，一旦服务在
    // handler 执行前重启，IndexDatabaseExists() 会因版本匹配返回 true，导致
    // 进入 CreateResumeHandler 但无 .old 目录可迁移，索引不会真正重建。
    if (type == IndexTask::Type::Create) {
        fmInfo() << "[TaskManager::startTask] Create task detected, clearing existing index status";
        if (m_context && m_context->stateStore()) {
            m_context->stateStore()->removeIndexStatusFile();
            m_context->stateStore()->setCreateFileListCache({});
            m_context->stateStore()->setCreateCheckpoint(0);
            m_context->stateStore()->setCreateInProgress(true);
        }
    }

    // 环境检查：如果当前环境不允许该分级任务运行，入队等待而非直接执行
    {
        TaskQueueItem item;
        item.type = type;
        item.grade = grade;
        item.forceBypass = forceBypass;
        item.path = primaryPath;
        item.pathList = pathList;
        if (tryEnqueueIfBlocked(grade, forceBypass, item))
            return true;
    }

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
        item.grade = grade;
        item.forceBypass = forceBypass;
        item.path = primaryPath;   // 保留主路径用于兼容现有代码
        item.pathList = pathList;   // 保存所有路径
        taskQueue.enqueue(item);

        fmInfo() << "[TaskManager::startTask] Task queued successfully, will execute after current task stops";
        // 返回true表示任务已经被接受，将在当前任务停止后执行
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "[TaskManager::startTask] Starting new task immediately - paths:" << pathList.size()
             << "primary:" << primaryPath << "type:" << static_cast<int>(type);

    // 清除队列中与新任务同类型同路径的冗余任务，避免暂停→手动更新→完成后重复执行
    removeDuplicateFullScanTasks(type, pathList);

    // force-bypass 全量任务覆盖了所有增量变更，清除队列中所有被阻塞的任务。
    // 否则任务完成后队列非空 → finalizeIndexState 不设 Clean → 状态卡在 Waiting*。
    // 全量扫描覆盖了所有路径，包括 FileList 任务的 path 格式（FileList-yyyymmdd-hhmmss）
    // 和 MoveFileList 任务的 path 格式（MoveList-yyyymmdd-hhmmss），这些路径无法通过
    // 路径匹配来识别，因此直接清空整个队列。
    if (forceBypass && isFullScanTask(type) && !taskQueue.isEmpty()) {
        fmInfo() << "[TaskManager::startTask] Clearing" << taskQueue.size()
                 << "blocked task(s) from queue - force-bypass full-scan supersedes all pending work";
        taskQueue.clear();
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
    IndexTask *task = new IndexTask(type, primaryPath, [handler, pathList](const QString &, TaskState &state) -> HandlerResult {
        fmDebug() << "[TaskManager::startTask] Executing task handler for" << pathList.size() << "paths";
        HandlerResult finalResult { true, false, false, false };

        for (const auto &path : pathList) {
            if (!state.isRunning()) {
                fmInfo() << "[TaskManager::startTask] Task execution interrupted during path processing";
                finalResult.interrupted = true;
                break;
            }

            fmDebug() << "[TaskManager::startTask] Processing path:" << path;
            HandlerResult pathResult = handler(path, state);

            if (!pathResult.success) {
                fmWarning() << "[TaskManager::startTask] Path processing failed:" << path;
                finalResult.success = false;
            }

            if (pathResult.fatal) {
                fmCritical() << "[TaskManager::startTask] Fatal error occurred during path processing:" << path;
                finalResult.fatal = true;
                break;
            }

            if (pathResult.interrupted) {
                fmInfo() << "[TaskManager::startTask] Path processing interrupted:" << path;
                finalResult.interrupted = true;
                break;
            }

            if (pathResult.useAnything) {
                fmInfo() << "[TaskManager::startTask] Using ANYTHING for file discovery, skipping remaining paths";
                break;
            }

            if (pathResult.indexChanged) {
                finalResult.indexChanged = true;
            }
        }

        fmInfo() << "[TaskManager::startTask] Task handler execution completed - success:" << finalResult.success
                 << "interrupted:" << finalResult.interrupted << "fatal:" << finalResult.fatal;
        return finalResult;
    });

    launchTask(task, grade, forceBypass);
    fmInfo() << "[TaskManager::startTask] Task started successfully in worker thread";
    return true;
}

bool TaskManager::startFileListTask(IndexTask::Type type, const QStringList &fileList)
{
    fmInfo() << "[TaskManager::startFileListTask] File list task request - type:" << static_cast<int>(type)
             << "files:" << fileList.size();

    if (fileList.isEmpty()) {
        fmWarning() << "[TaskManager::startFileListTask] Cannot start task - file list is empty";
        return false;
    }

    // RemoveFileList 始终视为轻量任务，与大小和总数无关
    const IndexTask::Grade grade = (type == IndexTask::Type::RemoveFileList)
            ? IndexTask::Grade::Light
            : gradeFileListTask(fileList);

    // RemoveFileList 是纯索引元数据删除操作，开销极低且需要保持索引与现实同步，
    // 不受环境门槛限制（电池/节能/空闲），始终立即执行
    const bool bypassEnv = (type == IndexTask::Type::RemoveFileList);

    // 环境检查：如果当前环境不允许该分级任务运行，入队等待而非直接执行
    {
        TaskQueueItem item;
        item.type = type;
        item.grade = grade;
        item.forceBypass = bypassEnv;
        item.path = QString("FileList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        item.fileList = fileList;
        if (tryEnqueueIfBlocked(grade, bypassEnv, item))
            return true;
    }

    // 如果当前有任务在运行，将新任务加入队列
    if (hasRunningTask() || currentTask) {
        fmInfo() << "[TaskManager::startFileListTask] Current task running, queuing file list task with"
                 << fileList.size() << "files";

        TaskQueueItem item;
        item.type = type;
        item.grade = grade;
        item.forceBypass = bypassEnv;
        item.path = QString("FileList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        item.fileList = fileList;
        taskQueue.enqueue(item);

        fmDebug() << "[TaskManager::startFileListTask] File list task queued successfully with grade:" << static_cast<int>(item.grade);
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "[TaskManager::startFileListTask] Starting file list task immediately - files:" << fileList.size()
             << "type:" << static_cast<int>(type);

    // 获取对应的任务处理器
    TaskHandler handler;
    switch (type) {
    case IndexTask::Type::CreateFileList:
        handler = TaskHandlers::CreateOrUpdateFileListHandler(*m_context, fileList);
        break;
    case IndexTask::Type::UpdateFileList:
        handler = TaskHandlers::CreateOrUpdateFileListHandler(*m_context, fileList);
        break;
    case IndexTask::Type::RemoveFileList:
        handler = TaskHandlers::RemoveFileListHandler(*m_context, fileList);
        break;
    default:
        fmCritical() << "[TaskManager::startFileListTask] Unknown file list task type:" << static_cast<int>(type);
        return false;
    }

    QString pathId = QString("FileList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
    launchTask(new IndexTask(type, pathId, handler), grade, bypassEnv);
    fmDebug() << "[TaskManager::startFileListTask] File list task started successfully in worker thread";
    return true;
}

bool TaskManager::startFileMoveTask(const QHash<QString, QString> &movedFiles)
{
    fmInfo() << "[TaskManager::startFileMoveTask] File move task request - moves:" << movedFiles.size();

    if (movedFiles.isEmpty()) {
        fmWarning() << "[TaskManager::startFileMoveTask] Cannot start task - moved files list is empty";
        return false;
    }

    const QStringList compensationPaths = applyDirectoryMovePlans(movedFiles);

    // MoveFileList 是纯索引路径更新操作，开销极低且需要保持索引与现实同步，
    // 不受环境门槛限制（电池/节能/空闲），始终立即执行
    {
        TaskQueueItem item;
        item.type = IndexTask::Type::MoveFileList;
        item.grade = IndexTask::Grade::Light;
        item.forceBypass = true;
        item.path = QString("MoveList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        item.movedFiles = movedFiles;
        if (tryEnqueueIfBlocked(IndexTask::Grade::Light, true, item)) {
            enqueueCompensationTask(compensationPaths);
            return true;
        }
    }

    // 如果当前有任务在运行，将新任务加入队列
    if (hasRunningTask() || currentTask) {
        fmInfo() << "[TaskManager::startFileMoveTask] Current task running, queuing file move task with"
                 << movedFiles.size() << "moves";

        TaskQueueItem item;
        item.type = IndexTask::Type::MoveFileList;
        item.grade = IndexTask::Grade::Light;
        item.forceBypass = true;
        item.path = QString("MoveList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        item.movedFiles = movedFiles;
        taskQueue.enqueue(item);
        enqueueCompensationTask(compensationPaths);

        fmDebug() << "[TaskManager::startFileMoveTask] File move task queued successfully";
        return true;
    }

    // 正常启动任务流程
    fmInfo() << "[TaskManager::startFileMoveTask] Starting file move task immediately - moves:" << movedFiles.size();

    TaskHandler handler = TaskHandlers::MoveFileListHandler(*m_context, movedFiles);
    if (!handler) {
        fmCritical() << "[TaskManager::startFileMoveTask] Failed to create move file list handler";
        return false;
    }

    QString pathId = QString("MoveList-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
    launchTask(new IndexTask(IndexTask::Type::MoveFileList, pathId, handler), IndexTask::Grade::Light, true);
    fmDebug() << "[TaskManager::startFileMoveTask] File move task started successfully in worker thread";

    enqueueCompensationTask(compensationPaths);
    return true;
}

TaskHandler TaskManager::getTaskHandler(IndexTask::Type type)
{
    if (!m_context)
        return nullptr;

    switch (type) {
    case IndexTask::Type::Create:
        return TaskHandlers::CreateIndexHandler(*m_context);
    case IndexTask::Type::Update:
        if (m_context->stateStore() && m_context->stateStore()->isCreateInProgress())
            return TaskHandlers::CreateResumeHandler(*m_context);
        return TaskHandlers::UpdateIndexHandler(*m_context);
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

void TaskManager::onTaskPaused(IndexTask::Type type, HandlerResult result)
{
    if (!currentTask) {
        fmWarning() << "[TaskManager::onTaskPaused] Received paused signal but no current task exists";
        return;
    }

    const QString taskPath = currentTask->taskPath();
    const IndexTask::Grade grade = currentTask->grade();
    fmInfo() << "[TaskManager::onTaskPaused] Task paused - type:" << static_cast<int>(type)
             << "path:" << taskPath << "grade:" << static_cast<int>(grade);

    // Save remaining work to queue
    TaskQueueItem item;
    item.grade = grade;

    if (!result.remainingFiles.isEmpty()) {
        // File list task: save remaining files
        item.type = IndexTask::Type::UpdateFileList;
        item.fileList = result.remainingFiles;
        item.path = QString("Resumed-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
    } else {
        // Full-scan task (Heavy or Light/Medium): resume as Update (skip already indexed files)
        item.type = IndexTask::Type::Update;
        item.path = taskPath;
        item.pathList = QStringList { taskPath };
    }

    taskQueue.enqueue(item);

    cleanupTask();
    emit indexStatusChanged(currentIndexStatus(), gradeToString(grade));

    // Try to schedule the next runnable task
    schedule();
}

void TaskManager::onEnvStateChanged(const EnvState &env)
{
    fmInfo() << "[TaskManager::onEnvStateChanged] Environment changed -"
             << "battery:" << env.onBattery << "powerSave:" << env.powerSaveMode << "idle:" << env.idle;

    // 1. Check if current running task needs to be paused
    //    forceBypass only bypasses the start gate. At runtime, only
    //    RemoveFileList/MoveFileList are exempt from env-based pausing.
    if (currentTask) {
        const auto type = currentTask->taskType();
        const bool exempt = (type == IndexTask::Type::RemoveFileList
                             || type == IndexTask::Type::MoveFileList);
        if (!exempt && !canRun(currentTask->grade(), false, env)) {
            fmInfo() << "[TaskManager::onEnvStateChanged] Current task can no longer run, pausing";
            pauseCurrentTask();
            return;   // schedule() will be called from onTaskPaused, which emits
        }
    }

    // 2. Try to schedule next task if worker is idle
    if (!hasRunningTask()) {
        schedule();
    }

    // 3. Always emit status change so clients can update their display.
    //    Even with no running/queued task, the status may have changed
    //    (e.g., "Idle" → "WaitingPowerSave" when entering power-save mode
    //    with a dirty index that needs updating).
    emit indexStatusChanged(currentIndexStatus(),
                            gradeToString(currentOrQueuedGrade().value_or(IndexTask::Grade::None)));
}

bool TaskManager::canRun(IndexTask::Grade grade, bool forceBypass, const EnvState &env) const
{
    if (forceBypass)
        return true;

    switch (grade) {
    case IndexTask::Grade::Manual:
    case IndexTask::Grade::Light:
        return !env.powerSaveMode;
    case IndexTask::Grade::Medium:
    case IndexTask::Grade::Heavy:
        return !env.onBattery && !env.powerSaveMode && env.idle;
    default:
        return false;
    }
    }

void TaskManager::pauseCurrentTask()
{
    if (currentTask) {
        fmInfo() << "[TaskManager::pauseCurrentTask] Requesting pause for current task - type:"
                 << static_cast<int>(currentTask->taskType()) << "path:" << currentTask->taskPath();
        currentTask->requestPause();
    }
}

void TaskManager::launchTask(IndexTask *task, IndexTask::Grade grade, bool forceBypass)
{
    Q_ASSERT(!currentTask);
    currentTask = task;
    currentTask->setGrade(grade);
    currentTask->setForceBypass(forceBypass);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::paused, this, &TaskManager::onTaskPaused, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);
    workerThread.start();

    if (m_context && m_context->stateStore())
        m_context->stateStore()->setIndexState(IndexUtility::IndexState::Dirty);

    emit startTaskInThread();
    emit indexStatusChanged("Running", gradeToString(grade));
}

bool TaskManager::tryEnqueueIfBlocked(IndexTask::Grade grade, bool forceBypass, const TaskQueueItem &item)
{
    const EnvState env = EnvDetector::instance().currentState();
    if (canRun(grade, forceBypass, env))
        return false;

    fmInfo() << "[TaskManager] Environment not suitable for grade" << static_cast<int>(grade)
             << ", queuing task - battery:" << env.onBattery
             << "powerSave:" << env.powerSaveMode << "idle:" << env.idle;
    taskQueue.enqueue(item);

    // A task being blocked means there's pending index work that hasn't been
    // reflected in the index yet.  Mark the state Dirty so that:
    // 1. A kill -9 (no cleanup) still leaves Dirty on disk → next startup
    //    detects it and runs a recovery Update.
    // 2. The indexStatusChanged signal (below) carries the correct status.
    if (m_context && m_context->stateStore())
        m_context->stateStore()->setIndexState(IndexUtility::IndexState::Dirty);

    // Notify clients immediately — the status changed from whatever it was
    // (e.g., "Idle") to a waiting state because a task is now blocked.
    emit indexStatusChanged(currentIndexStatus(),
                            gradeToString(currentOrQueuedGrade().value_or(IndexTask::Grade::None)));
    return true;
}

void TaskManager::schedule()
{
    if (hasRunningTask() || currentTask) {
        fmDebug() << "[TaskManager::schedule] Worker busy, skipping schedule";
        return;
    }

    if (taskQueue.isEmpty()) {
        fmDebug() << "[TaskManager::schedule] No tasks in queue";
        return;
    }

    const EnvState env = EnvDetector::instance().currentState();

    int bestIdx = -1;
    int bestPri = -1;
    for (int i = 0; i < taskQueue.size(); ++i) {
        const auto &item = taskQueue[i];
        if (canRun(item.grade, item.forceBypass, env)) {
            int pri = gradePriority(item.grade);
            if (pri > bestPri) {
                bestPri = pri;
                bestIdx = i;
            }
        }
    }

    if (bestIdx < 0) {
        fmInfo() << "[TaskManager::schedule] No runnable tasks in queue (blocked by environment)";
        return;
    }

    TaskQueueItem item = taskQueue.takeAt(bestIdx);
    fmInfo() << "[TaskManager::schedule] Scheduling task - type:" << static_cast<int>(item.type)
             << "grade:" << static_cast<int>(item.grade) << "remaining:" << taskQueue.count();
    startQueuedTask(item);
}

void TaskManager::startQueuedTask(const TaskQueueItem &item)
{
    if (item.type == IndexTask::Type::CreateFileList
        || item.type == IndexTask::Type::UpdateFileList
        || item.type == IndexTask::Type::RemoveFileList) {
        startFileListTask(item.type, item.fileList);
    } else if (item.type == IndexTask::Type::MoveFileList) {
        startFileMoveTask(item.movedFiles);
    } else if (!item.pathList.isEmpty()) {
        startTask(item.type, item.pathList, item.grade, item.forceBypass);
    } else {
        startTask(item.type, QStringList { item.path }, item.grade, item.forceBypass);
    }
}

QString TaskManager::gradeToString(IndexTask::Grade grade)
{
    switch (grade) {
    case IndexTask::Grade::None:   return "none";
    case IndexTask::Grade::Light:  return "light";
    case IndexTask::Grade::Medium: return "medium";
    case IndexTask::Grade::Heavy:  return "heavy";
    case IndexTask::Grade::Manual: return "manual";
    default: return "unknown";
    }
}

int TaskManager::gradePriority(IndexTask::Grade grade)
{
    switch (grade) {
    case IndexTask::Grade::Manual: return 4;
    case IndexTask::Grade::Heavy:  return 3;
    case IndexTask::Grade::Medium: return 2;
    case IndexTask::Grade::Light:  return 1;
    default: return 0;
    }
}

std::optional<IndexTask::Grade> TaskManager::currentTaskGrade() const
{
    if (!hasRunningTask())
        return std::nullopt;
    return currentTask->grade();
}

std::optional<IndexTask::Grade> TaskManager::currentOrQueuedGrade() const
{
    if (hasRunningTask() && currentTask)
        return currentTask->grade();
    if (!taskQueue.isEmpty())
        return taskQueue.head().grade;
    return std::nullopt;
}

QString TaskManager::currentIndexStatus() const
{
    if (currentTask)
        return "Running";

    if (!hasRunningTask() && !currentTask && !taskQueue.isEmpty()) {
        // Tasks queued but not running → blocked by environment.
        const EnvState env = EnvDetector::instance().currentState();
        const IndexTask::Grade headGrade = taskQueue.head().grade;

        if (canRun(headGrade, taskQueue.head().forceBypass, env))
            return "Running";

        // Distinguish by the first unsatisfied condition.
        // The order matters: battery is the most restrictive, then power-save, then idle.
        // Light tasks only require !powerSaveMode, so they won't trigger WaitingPower/WaitingIdle.
        if (env.onBattery && headGrade != IndexTask::Grade::Light)
            return "WaitingPower";
        if (env.powerSaveMode)
            return "WaitingPowerSave";
        if (!env.idle && headGrade != IndexTask::Grade::Light)
            return "WaitingIdle";

        // Should not reach here: canRun returned false but no known condition matched.
        // This can only happen with an unknown grade (None) — treat as Idle.
        fmWarning() << "[TaskManager::currentIndexStatus] Unreachable: canRun=false but no condition matched"
                    << "grade:" << static_cast<int>(headGrade)
                    << "battery:" << env.onBattery
                    << "powerSave:" << env.powerSaveMode
                    << "idle:" << env.idle;
        return "Idle";
    }

    // No running task, no queued tasks
    if (m_lastTaskFailed)
        return "Failed";

    // There's pending work when: the state is dirty (unsynced changes), a
    // create was interrupted (needs resuming), or the database doesn't exist
    // or its version is incompatible (needs creation or upgrade).
    //
    // Even a "clean" state needs an upgrade when the stored version doesn't
    // match the runtime version — the old status.json still says "clean" but
    // a rebuild is required.  Without this check the user would see "Idle"
    // until the silent-start timer fires, instead of "WaitingUpgrade".
    //
    // The status shown to the user depends on the *effective grade* of the
    // pending work, mirroring gradeUpdateTask() / handleSlientStart() logic:
    //
    //   isCreateInProgress() → Heavy (resume interrupted full build)
    //   DB doesn't exist      → Heavy (fresh Create / version mismatch)
    //   Otherwise             → Light  (开机全盘扫描对比 — incremental update)
    //
    // Display rules (per spec):
    //
    //   Heavy: always show "WaitingUpgrade" during the update interval —
    //   the user needs to know a major rebuild is pending.  When the timer
    //   fires and the task is blocked, it lands in the queue and the
    //   "queue not empty" branch above reports the specific waiting state
    //   (WaitingPower / WaitingPowerSave / WaitingIdle).
    //
    //   Light: show "Idle" (as if completed) — minor pending changes will be
    //   silently picked up when the update timer fires.  The only exception is
    //   power-save mode, which blocks even Light tasks and is worth telling
    //   the user about.
    if (m_context && m_context->stateStore()) {
        const bool createInProgress = m_context->stateStore()->isCreateInProgress();
        const bool dbExists = m_context->profile().isIndexAvailable()
                            && m_context->stateStore()->isCompatibleVersion()
                            && !m_context->stateStore()->getLastUpdateTime().isEmpty();
        const bool hasPendingWork = !m_context->stateStore()->isCleanState()
                                  || createInProgress || !dbExists;

        if (hasPendingWork) {
            const bool heavy = createInProgress || !dbExists;
            if (heavy) {
                return "WaitingUpgrade";
            } else {
                const EnvState env = EnvDetector::instance().currentState();
                if (env.powerSaveMode)
                    return "WaitingPowerSave";
                return "Idle";
            }
        }
    }

    return "Idle";
}

void TaskManager::onTaskFinished(IndexTask::Type type, HandlerResult result)
{
    if (!currentTask) {
        fmWarning() << "[TaskManager::onTaskFinished] Received task finished signal but no current task exists";
        return;
    }

    const QString taskPath = currentTask->taskPath();
    fmInfo() << "[TaskManager::onTaskFinished] Task finished - type:" << static_cast<int>(type)
             << "path:" << taskPath << "success:" << result.success << "interrupted:" << result.interrupted;

    // 处理索引损坏：若已启动重建任务则提前返回
    if (handleCorruptedIndex(type, result, taskPath))
        return;

    fmDebug() << "[TaskManager::onTaskFinished] Task" << typeToString(type) << "for path" << taskPath
              << (result.success ? "completed successfully" : "failed");

    handleRootPathFailure(result.success, result.interrupted, taskPath);
    updateIndexStatusOnSuccess(type, result);

    // Track failure state for status reporting (ignore interrupted/paused)
    if (!result.interrupted && !result.paused)
        m_lastTaskFailed = !result.success;

    emit taskFinished(typeToString(type), taskPath, result.success);
    cleanupTask();

    emit indexStatusChanged(currentIndexStatus(), gradeToString(IndexTask::Grade::None));

    schedule();

    if (!hasRunningTask()) {
        fmDebug() << "[TaskManager::onTaskFinished] No more runnable tasks";
        finalizeIndexState(type, result);
    }
}

bool TaskManager::handleCorruptedIndex(IndexTask::Type type, const HandlerResult &result, const QString &taskPath)
{
    if (result.success || type != IndexTask::Type::Update)
        return false;
    if (!currentTask->isIndexCorrupted()) {
        fmInfo() << "[TaskManager::onTaskFinished] Update task failed but index is not corrupted, skipping rebuild - path:" << taskPath;
        return false;
    }

    fmWarning() << "[TaskManager::onTaskFinished] Update task failed due to index corruption, attempting rebuild - path:" << taskPath;

    if (m_context && m_context->stateStore())
        m_context->stateStore()->clearIndexDirectory();

    cleanupTask();

    if (!taskQueue.isEmpty() && taskQueue.head().pathList.contains(taskPath)) {
        fmInfo() << "[TaskManager::onTaskFinished] Found queued task containing corrupted path, letting queue handle rebuild";
    } else {
        fmInfo() << "[TaskManager::onTaskFinished] Starting rebuild task for corrupted index - path:" << taskPath;
        if (startTask(IndexTask::Type::Create, taskPath))
            return true;   // 重建任务已启动
        fmCritical() << "[TaskManager::onTaskFinished] Failed to start rebuild task for path:" << taskPath;
    }

    return false;
}

void TaskManager::handleRootPathFailure(bool success, bool interrupted, const QString &taskPath)
{
    if (success || interrupted)
        return;
    if (!m_context || !m_context->profile().isPathInScope(taskPath))
        return;

    fmWarning() << "[TaskManager::onTaskFinished] Root indexing failed, clearing status - path:" << taskPath;
    if (m_context->stateStore())
        m_context->stateStore()->removeIndexStatusFile();
}

void TaskManager::updateIndexStatusOnSuccess(IndexTask::Type type, const HandlerResult &result)
{
    if (!result.success)
        return;
    if (result.interrupted && type != IndexTask::Type::Create)
        return;
    if (!result.indexChanged && !isFullScanTask(type)) {
        fmDebug() << "[TaskManager::onTaskFinished] Task completed with no index changes, skipping status update";
        return;
    }

    fmDebug() << "[TaskManager::onTaskFinished] Task completed with actual index changes, updating index status";
    if (!m_context || !m_context->stateStore())
        return;

    // Full-scan tasks (Create/Update) update version number;
    // Incremental tasks only update last update time to avoid version mismatch
    // when recovery from a previous interrupted full-scan task is pending.
    if (isFullScanTask(type)) {
        m_context->stateStore()->saveIndexStatus(QDateTime::currentDateTime());
    } else {
        m_context->stateStore()->saveLastUpdateTime(QDateTime::currentDateTime());
    }
}

void TaskManager::finalizeIndexState(IndexTask::Type type, const HandlerResult &result)
{
    // Only set Clean state when:
    // 1. Task completed successfully without interruption
    // 2. No recovery is pending (or this is the recovery task completing)
    // 3. No tasks are still queued (otherwise there's pending work)
    if (!result.success || result.interrupted)
        return;

    if (!taskQueue.isEmpty()) {
        fmInfo() << "[TaskManager::onTaskFinished] Tasks still queued, keeping Dirty state";
        return;
    }

    if (isFullScanTask(type)) {
        m_recoveryPending = false;
        if (m_context && m_context->stateStore()) {
            m_context->stateStore()->setIndexState(IndexUtility::IndexState::Clean);
            m_context->stateStore()->setCreateInProgress(false);
            m_context->stateStore()->setCreateFileListCache({});
            m_context->stateStore()->setCreateCheckpoint(0);
        }
        fmInfo() << "[TaskManager::onTaskFinished] Full-scan task completed, index state set to clean";
    } else if (!m_recoveryPending) {
        if (m_context && m_context->stateStore())
            m_context->stateStore()->setIndexState(IndexUtility::IndexState::Clean);
        fmInfo() << "[TaskManager::onTaskFinished] Incremental task completed, index state set to clean";
    } else {
        fmInfo() << "[TaskManager::onTaskFinished] Incremental task completed but recovery is pending, keeping Dirty state";
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

void TaskManager::setRecoveryPending(bool pending)
{
    m_recoveryPending = pending;
    fmInfo() << "[TaskManager] Recovery pending state set to:" << pending;
}

bool TaskManager::isRecoveryPending() const
{
    return m_recoveryPending;
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

bool TaskManager::isFullScanTask(IndexTask::Type type) const
{
    return type == IndexTask::Type::Create || type == IndexTask::Type::Update;
}

void TaskManager::removeDuplicateFullScanTasks(IndexTask::Type type, const QStringList &pathList)
{
    if (!isFullScanTask(type))
        return;

    const QSet<QString> newPathSet(pathList.begin(), pathList.end());
    for (int i = taskQueue.size() - 1; i >= 0; --i) {
        const auto &item = taskQueue[i];
        if (!isFullScanTask(item.type))
            continue;

        const QStringList itemPaths = item.pathList.isEmpty()
                ? QStringList { item.path }
                : item.pathList;
        const QSet<QString> itemPathSet(itemPaths.begin(), itemPaths.end());
        if (newPathSet == itemPathSet) {
            fmInfo() << "[TaskManager] Removing duplicate full-scan task from queue - type:"
                     << static_cast<int>(item.type) << "path:" << item.path;
            taskQueue.removeAt(i);
        }
    }
}

IndexTask::Grade TaskManager::gradeFileListTask(const QStringList &fileList) const
{
    auto &config = TextIndexConfig::instance();
    int countThreshold = isOcrProfile()
            ? config.lightIncrementOcrFileCountThreshold()
            : config.lightIncrementFileCountThreshold();

    if (fileList.size() > countThreshold)
        return IndexTask::Grade::Medium;

    qint64 sizeThreshold = config.lightIncrementSizeThresholdMB() * 1024 * 1024;
    qint64 totalSize = 0;
    for (const auto &filePath : fileList) {
        QFileInfo info(filePath);
        totalSize += info.size();
        if (totalSize > sizeThreshold)
            return IndexTask::Grade::Medium;
    }
    return IndexTask::Grade::Light;
}

IndexTask::Grade TaskManager::gradeUpdateTask() const
{
    if (m_context && m_context->stateStore() && m_context->stateStore()->isCreateInProgress())
        return IndexTask::Grade::Heavy;
    return IndexTask::Grade::Light;
}

bool TaskManager::isOcrProfile() const
{
    return m_context && m_context->profile().type() == IndexProfile::Type::Ocr;
}

bool TaskManager::enqueueCompensationTask(const QStringList &paths)
{
    if (paths.isEmpty()) {
        return false;
    }

    taskQueue.enqueue(createCompensationTaskItem(paths));
    fmInfo() << "[TaskManager::enqueueCompensationTask] Queued directory compensation update for"
             << paths.size() << "path(s), primary:" << paths.first();
    return true;
}

QStringList TaskManager::applyDirectoryMovePlans(const QHash<QString, QString> &movedFiles)
{
    const QList<TaskQueueUtils::DirectoryMovePlan> plans = TaskQueueUtils::buildDirectoryMovePlans(movedFiles);
    if (plans.isEmpty()) {
        return {};
    }

    QStringList compensationPaths;

    for (const TaskQueueUtils::DirectoryMovePlan &plan : plans) {
        const bool rewroteQueuedTasks = TaskQueueUtils::rewriteQueuedTasksForDirectoryMove(taskQueue,
                                                                                           plan.fromPath,
                                                                                           plan.toPath);
        if (rewroteQueuedTasks) {
            fmInfo() << "[TaskManager::applyDirectoryMovePlans] Rewrote queued task paths for directory move:"
                     << plan.fromPath << "->" << plan.toPath;
        }

        if (!compensationPaths.contains(plan.toPath)) {
            compensationPaths.append(plan.toPath);
        }
    }

    return compensationPaths;
}
