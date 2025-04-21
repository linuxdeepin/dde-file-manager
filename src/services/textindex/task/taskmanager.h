// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "service_textindex_global.h"
#include "indextask.h"

#include <QObject>
#include <QThread>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager();

    bool startTask(IndexTask::Type type, const QString &path);
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
    void startPendingTaskIfAny();

    QThread workerThread;
    IndexTask *currentTask { nullptr };

    // 保存待执行的任务信息
    IndexTask::Type pendingTaskType;
    QString pendingTaskPath;
    bool hasPendingTask { false };

    static QString typeToString(IndexTask::Type type);
};

SERVICETEXTINDEX_END_NAMESPACE
#endif   // TASKMANAGER_H
