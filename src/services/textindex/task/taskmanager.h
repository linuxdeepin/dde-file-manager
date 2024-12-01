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
    QString getLastUpdateTime() const;

Q_SIGNALS:
    void taskFinished(const QString &type, const QString &path, bool success);
    void taskProgressChanged(const QString &type, const QString &path, qint64 count);
    void startTaskInThread();

private Q_SLOTS:
    void onTaskProgress(IndexTask::Type type, qint64 count);
    void onTaskFinished(IndexTask::Type type, bool success);

private:
    void cleanupTask();
    void clearIndexDirectory();

    QThread workerThread;
    IndexTask *currentTask { nullptr };

    static QString typeToString(IndexTask::Type type);
};

SERVICETEXTINDEX_END_NAMESPACE
#endif   // TASKMANAGER_H
