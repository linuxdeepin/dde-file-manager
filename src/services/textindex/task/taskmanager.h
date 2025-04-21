// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
    QStringList fileList;   // 仅在文件列表类型任务中使用
};

class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager();

    // 原有的基于路径的任务启动方法
    bool startTask(IndexTask::Type type, const QString &path);

    // 新增的基于文件列表的任务启动方法
    bool startFileListTask(IndexTask::Type type, const QStringList &fileList);

    bool hasRunningTask() const;
    void stopCurrentTask();

Q_SIGNALS:
    void taskFinished(const QString &type, const QString &path, bool success);
    void taskProgressChanged(const QString &type, const QString &path, qint64 count);
    void startTaskInThread();

private Q_SLOTS:
    void onTaskProgress(IndexTask::Type type, qint64 count);
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
