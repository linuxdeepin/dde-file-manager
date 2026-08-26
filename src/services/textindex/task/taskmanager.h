// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "service_textindex_global.h"
#include "core/indexcontext.h"
#include "indextask.h"

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QHash>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class EnvDetector;
struct EnvState;

// 任务队列项
struct TaskQueueItem
{
    IndexTask::Type type;
    IndexTask::Grade grade { IndexTask::Grade::None };   // 任务分级
    QString path;
    QStringList pathList;   // 当传入多个路径时使用
    QStringList fileList;   // 仅在文件列表类型任务中使用
    QStringList remainingFiles;   // 暂停任务的剩余文件列表
    QHash<QString, QString> movedFiles;  // 仅在移动任务中使用 (fromPath -> toPath)
    bool forceBypass { false };   // 手动绕过环境门槛
};

class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(const IndexContext *context, QObject *parent = nullptr);
    ~TaskManager();

    bool startTask(IndexTask::Type type, const QStringList &pathList,
                   IndexTask::Grade grade = IndexTask::Grade::None, bool forceBypass = false);
    bool startTask(IndexTask::Type type, const QString &path);

    bool startFileListTask(IndexTask::Type type, const QStringList &fileList);

    bool startFileMoveTask(const QHash<QString, QString> &movedFiles);

    bool hasRunningTask() const;
    bool hasQueuedTasks() const;
    void stopCurrentTask();

    std::optional<IndexTask::Type> currentTaskType() const;
    std::optional<QString> currentTaskPath() const;
    std::optional<IndexTask::Grade> currentTaskGrade() const;
    std::optional<IndexTask::Grade> currentOrQueuedGrade() const;
    QString currentIndexStatus() const;

    static QString gradeToString(IndexTask::Grade grade);

    // Recovery state management - used to prevent incremental tasks from
    // clearing Dirty state before recovery task completes
    void setRecoveryPending(bool pending);
    bool isRecoveryPending() const;

Q_SIGNALS:
    void taskFinished(const QString &type, const QString &path, bool success);
    void taskProgressChanged(const QString &type, const QString &path, qint64 count, qint64 total);
    void startTaskInThread();
    void indexStatusChanged(const QString &state, const QString &grade);

private Q_SLOTS:
    void onTaskProgress(IndexTask::Type type, qint64 count, qint64 total);
    void onTaskFinished(IndexTask::Type type, SERVICETEXTINDEX_NAMESPACE::HandlerResult result);
    void onTaskPaused(IndexTask::Type type, SERVICETEXTINDEX_NAMESPACE::HandlerResult result);
    void onEnvStateChanged(const EnvState &env);

private:
    void cleanupTask();
    void schedule();
    bool canRun(IndexTask::Grade grade, bool forceBypass, const EnvState &env) const;
    void pauseCurrentTask();
    void launchTask(IndexTask *task, IndexTask::Grade grade, bool forceBypass = false);
    bool tryEnqueueIfBlocked(IndexTask::Grade grade, bool forceBypass, const TaskQueueItem &item);
    void startQueuedTask(const TaskQueueItem &item);
    TaskHandler getTaskHandler(IndexTask::Type type);
    bool isFullScanTask(IndexTask::Type type) const;
    bool enqueueCompensationTask(const QStringList &paths);
    QStringList applyDirectoryMovePlans(const QHash<QString, QString> &movedFiles);
    void removeDuplicateFullScanTasks(IndexTask::Type type, const QStringList &pathList);

    // Task grading
    IndexTask::Grade gradeFileListTask(const QStringList &fileList) const;
    IndexTask::Grade gradeUpdateTask() const;
    bool isOcrProfile() const;

    // onTaskFinished sub-routines
    bool handleCorruptedIndex(IndexTask::Type type, const HandlerResult &result, const QString &taskPath);
    void handleRootPathFailure(bool success, bool interrupted, const QString &taskPath);
    void updateIndexStatusOnSuccess(IndexTask::Type type, const HandlerResult &result);
    void finalizeIndexState(IndexTask::Type type, const HandlerResult &result);

    static QString typeToString(IndexTask::Type type);
    static int gradePriority(IndexTask::Grade grade);

    const IndexContext *m_context { nullptr };
    QThread workerThread;
    IndexTask *currentTask { nullptr };

    // 保存待执行的任务信息
    QQueue<TaskQueueItem> taskQueue;

    // Recovery pending flag - set at service startup if Dirty state detected
    // Prevents incremental tasks from clearing Dirty state before recovery completes
    bool m_recoveryPending { false };
    bool m_lastTaskFailed { false };
};

SERVICETEXTINDEX_END_NAMESPACE
#endif   // TASKMANAGER_H
