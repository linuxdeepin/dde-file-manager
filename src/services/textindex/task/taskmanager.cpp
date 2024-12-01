// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmanager.h"

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
        registered = true;
        fmDebug() << "Meta types registered successfully";
    }
}

QString getConfigPath()
{
    return indexStorePath() + "/index_status.json";
}

void clearIndexStatus()
{
    QFile file(getConfigPath());
    if (file.exists()) {
        fmInfo() << "Clearing index status file:" << file.fileName()
                 << "[Clearing index status configuration]";
        file.remove();
    }
}

void saveIndexStatus(const QDateTime &lastUpdateTime)
{
    QJsonObject status;
    status["lastUpdateTime"] = lastUpdateTime.toString(Qt::ISODate);
    
    QJsonDocument doc(status);
    QFile file(getConfigPath());
    
    // 确保目录存在
    QDir().mkpath(QFileInfo(file).absolutePath());
    
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        fmInfo() << "Index status saved successfully:" << file.fileName()
                 << "lastUpdateTime:" << lastUpdateTime.toString(Qt::ISODate)
                 << "[Updated index status configuration]";
    } else {
        fmWarning() << "Failed to save index status to:" << file.fileName()
                    << "[Failed to write index status configuration]";
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

bool TaskManager::startTask(IndexTask::Type type, const QString &path)
{
    if (hasRunningTask()) {
        fmWarning() << "Cannot start new task, another task is running";
        return false;
    }

    fmInfo() << "Starting new task for path:" << path;

    // 如果是根目录的任务，清除状态文件
    if (path == "/") {
        fmInfo() << "Root path task detected, clearing existing index status"
                 << "[Initializing new root indexing task]";
        clearIndexStatus();
    }

    // 获取对应的任务处理器
    TaskHandler handler;
    switch (type) {
    case IndexTask::Type::Create:
        handler = TaskHandlers::CreateIndexHandler();
        break;
    case IndexTask::Type::Update:
        handler = TaskHandlers::UpdateIndexHandler();
        break;
    case IndexTask::Type::Remove:
        handler = TaskHandlers::RemoveIndexHandler();
        break;
    default:
        fmWarning() << "Unknown task type:" << static_cast<int>(type);
        return false;
    }

    currentTask = new IndexTask(type, path, handler);
    currentTask->moveToThread(&workerThread);

    connect(currentTask, &IndexTask::progressChanged, this, &TaskManager::onTaskProgress, Qt::QueuedConnection);
    connect(currentTask, &IndexTask::finished, this, &TaskManager::onTaskFinished, Qt::QueuedConnection);
    connect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start, Qt::QueuedConnection);
    workerThread.start();

    emit startTaskInThread();
    fmDebug() << "Task started in worker thread";
    return true;
}

QString TaskManager::typeToString(IndexTask::Type type)
{
    switch (type) {
    case IndexTask::Type::Create:
        return "create";
    case IndexTask::Type::Update:
        return "update";
    case IndexTask::Type::Remove:
        return "remove";
    default:
        return "unknown";
    }
}

void TaskManager::onTaskProgress(IndexTask::Type type, qint64 count)
{
    if (!currentTask) return;

    fmDebug() << "Task progress:" << type << count;
    emit taskProgressChanged(typeToString(type), currentTask->taskPath(), count);
}

void TaskManager::onTaskFinished(IndexTask::Type type, bool success)
{
    if (!currentTask) return;

    QString taskPath = currentTask->taskPath();
    fmInfo() << "Task" << typeToString(type) << "for path" << taskPath
             << (success ? "completed successfully" : "failed");

    // 如果是根目录的任务，更新状态文件
    if (taskPath == "/") {
        if (success) {
            fmInfo() << "Root indexing completed successfully, updating status"
                     << "[Root index task succeeded]";
            saveIndexStatus(QDateTime::currentDateTime());
        } else {
            fmInfo() << "Root indexing failed, clearing status"
                     << "[Root index task failed]";
            clearIndexStatus();
        }
    }

    emit taskFinished(typeToString(type), taskPath, success);
    cleanupTask();
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

void TaskManager::cleanupTask()
{
    if (currentTask) {
        fmDebug() << "Cleaning up task resources";
        disconnect(this, &TaskManager::startTaskInThread, currentTask, &IndexTask::start);
        currentTask->deleteLater();
        currentTask = nullptr;
    }
}

QString TaskManager::getLastUpdateTime() const
{
    QFile file(getConfigPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("lastUpdateTime")) {
            QDateTime time = QDateTime::fromString(obj["lastUpdateTime"].toString(), Qt::ISODate);
            return time.toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    return QString();
}
