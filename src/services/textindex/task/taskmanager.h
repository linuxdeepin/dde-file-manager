#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "../service_textindex_global.h"
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
    void createFailed();
    void createSuccessful();
    void createIndexCountChanged(int);
    void updateFailed();
    void updateSuccessful();
    void updateIndexCountChanged(int);

private Q_SLOTS:
    void onTaskProgress(IndexTask::Type type, int count);
    void onTaskFinished(IndexTask::Type type, bool success);

private:
    void cleanupTask();

    QThread workerThread;
    IndexTask *currentTask { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE
#endif // TASKMANAGER_H
