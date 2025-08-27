// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syncdbus.h"
#include "daemonplugin_core_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QMetaObject>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

DAEMONPCORE_USE_NAMESPACE

// Maximum number of concurrent sync operations
static constexpr int kMaxConcurrentSyncs = 4;

SyncDBus::SyncDBus(QObject *parent)
    : QObject(parent)
    , m_threadPool(new QThreadPool(this))
    , m_nextTaskId(1)
{
    // Configure thread pool for sync operations
    m_threadPool->setMaxThreadCount(kMaxConcurrentSyncs);
    
    fmInfo() << "SyncDBus service initialized with max" << kMaxConcurrentSyncs << "concurrent operations";
}

SyncDBus::~SyncDBus()
{
    fmInfo() << "SyncDBus service shutting down";
    
    // Wait for all tasks to complete before destruction
    m_threadPool->waitForDone(5000); // 5 second timeout
    
    // Clean up any remaining tasks
    for (auto task : m_activeTasks) {
        task->deleteLater();
    }
    m_activeTasks.clear();
}

int SyncDBus::SyncFS(const QString &path, const QVariantMap &options)
{
    if (path.isEmpty()) {
        fmWarning() << "SyncFS called with empty path";
        return -1;
    }

    // Validate path exists
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        fmWarning() << "SyncFS path does not exist:" << path;
        return -1;
    }

    // Generate unique task ID
    int taskId = generateTaskId();
    
    // Create sync task
    SyncTask *task = new SyncTask(taskId, path, options);
    task->setAutoDelete(false); // We manage the lifetime manually
    
    // Connect task completion signal
    connect(task, &SyncTask::taskCompleted, this, &SyncDBus::onSyncTaskCompleted, Qt::QueuedConnection);
    
    // Add to active tasks
    m_activeTasks.insert(taskId, task);
    
    // Start the task
    m_threadPool->start(task);
    
    fmInfo() << "Started sync operation for path:" << path << "with task ID:" << taskId;
    
    return taskId;
}

QVariantMap SyncDBus::GetSyncStatus()
{
    QVariantMap status;
    
    status["activeTaskCount"] = m_activeTasks.size();
    status["maxConcurrentTasks"] = kMaxConcurrentSyncs;
    status["threadPoolActiveThreads"] = m_threadPool->activeThreadCount();
    
    QStringList activePaths;
    for (auto task : m_activeTasks) {
        activePaths.append(task->path());
    }
    status["activePaths"] = activePaths;
    
    return status;
}

void SyncDBus::onSyncTaskCompleted(SyncTask *task)
{
    if (!task) {
        fmWarning() << "Received null task in completion handler";
        return;
    }

    int taskId = task->taskId();
    QString path = task->path();
    
    // Remove from active tasks
    m_activeTasks.remove(taskId);
    
    // Emit appropriate signal based on task result
    if (task->isSuccessful()) {
        fmInfo() << "Sync operation completed successfully for path:" << path << "task ID:" << taskId;
        emit SyncCompleted(taskId, path);
    } else {
        QString errorMsg = task->errorMessage();
        fmWarning() << "Sync operation failed for path:" << path << "task ID:" << taskId << "error:" << errorMsg;
        emit SyncFailed(taskId, path, errorMsg);
    }
    
    // Clean up task
    task->deleteLater();
}

int SyncDBus::generateTaskId()
{
    return m_nextTaskId.fetchAndAddAcquire(1);
}

// SyncTask implementation

SyncTask::SyncTask(int taskId, const QString &path, const QVariantMap &options, QObject *parent)
    : QObject(parent)
    , m_taskId(taskId)
    , m_path(path)
    , m_options(options)
    , m_success(false)
{
}

void SyncTask::run()
{
    fmDebug() << "Starting sync task" << m_taskId << "for path:" << m_path;
    
    // Resolve the actual filesystem path
    QString resolvedPath = QFileInfo(m_path).absoluteFilePath();
    
    // Open the path (works for both files and directories)
    int fd = open(resolvedPath.toLocal8Bit().constData(), O_RDONLY);
    
    if (fd == -1) {
        m_errorMessage = QString("Failed to open path: %1 (errno: %2)").arg(resolvedPath).arg(strerror(errno));
        fmWarning() << "Sync task" << m_taskId << "failed:" << m_errorMessage;
        emit taskCompleted(this);
        return;
    }
    
    // Perform the sync operation
    int result = syncfs(fd);
    close(fd);
    
    if (result == 0) {
        m_success = true;
        fmDebug() << "Sync task" << m_taskId << "completed successfully for path:" << m_path;
    } else {
        m_errorMessage = QString("syncfs failed for path: %1 (errno: %2)").arg(resolvedPath).arg(strerror(errno));
        fmWarning() << "Sync task" << m_taskId << "failed:" << m_errorMessage;
    }
    
    // Emit completion signal
    emit taskCompleted(this);
} 