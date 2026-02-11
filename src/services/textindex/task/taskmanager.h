// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "service_textindex_global.h"
#include "indextask.h"

#include <QObject>
#include <QThread>
#include <QQueue>

SERVICETEXTINDEX_BEGIN_NAMESPACE

// 任务队列项
struct TaskQueueItem
{
    IndexTask::Type type;
    QString path;
    QStringList pathList;   // 当传入多个路径时使用
    QStringList fileList;   // 仅在文件列表类型任务中使用
    QHash<QString, QString> movedFiles;  // 仅在移动任务中使用 (fromPath -> toPath)
    bool silent { false };
};

class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager();

    bool startTask(IndexTask::Type type, const QStringList &pathList, bool silent = false);
    bool startTask(IndexTask::Type type, const QString &path, bool silent = false);

    bool startFileListTask(IndexTask::Type type, const QStringList &fileList, bool silent = false);

    bool startFileMoveTask(const QHash<QString, QString> &movedFiles, bool silent = false);

    bool hasRunningTask() const;
    bool hasQueuedTasks() const;
    void stopCurrentTask();

    std::optional<IndexTask::Type> currentTaskType() const;
    std::optional<QString> currentTaskPath() const;

Q_SIGNALS:
    void taskFinished(const QString &type, const QString &path, bool success);
    void taskProgressChanged(const QString &type, const QString &path, qint64 count, qint64 total);
    void startTaskInThread();

private Q_SLOTS:
    void onTaskProgress(IndexTask::Type type, qint64 count, qint64 total);
    void onTaskFinished(IndexTask::Type type, SERVICETEXTINDEX_NAMESPACE::HandlerResult result);

private:
    void cleanupTask();
    bool startNextTask();
    TaskHandler getTaskHandler(IndexTask::Type type);

    QThread workerThread;
    IndexTask *currentTask { nullptr };

    // 保存待执行的任务信息
    QQueue<TaskQueueItem> taskQueue;

    static QString typeToString(IndexTask::Type type);
};

SERVICETEXTINDEX_END_NAMESPACE
#endif   // TASKMANAGER_H
