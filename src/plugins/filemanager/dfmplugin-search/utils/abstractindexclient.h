// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTINDEXCLIENT_H
#define ABSTRACTINDEXCLIENT_H

#include "dfmplugin_search_global.h"
#include "indexclientdescriptor.h"

#include <QObject>
#include <QDBusPendingCallWatcher>
#include <memory>

class QDBusAbstractInterface;

DPSEARCH_BEGIN_NAMESPACE

class AbstractIndexClient : public QObject
{
    Q_OBJECT

public:
    enum class TaskType {
        Create,
        Update,
        CreateFileList,
        UpdateFileList,
        RemoveFileList,
        MoveFileList,
    };
    Q_ENUM(TaskType)

    enum class ServiceStatus {
        Available,
        Unavailable,
        Error
    };
    Q_ENUM(ServiceStatus)

    explicit AbstractIndexClient(IndexClientDescriptor descriptor, QObject *parent = nullptr);
    ~AbstractIndexClient() override;

    void startTask(TaskType type, const QStringList &paths);
    void checkIndexExists();
    void checkServiceStatus();
    void checkHasRunningRootTask();
    void checkHasRunningTask();
    void getLastUpdateTime();
    void setEnable(bool enabled);

Q_SIGNALS:
    void taskStarted(TaskType type, const QString &path);
    void taskFinished(TaskType type, const QString &path, bool success);
    void taskFailed(TaskType type, const QString &path, const QString &error);
    void taskProgressChanged(TaskType type, const QString &path, qlonglong count, qlonglong total);
    void indexExistsResult(bool exists, bool success);
    void serviceStatusResult(ServiceStatus status);
    void hasRunningTaskResult(bool running, bool success);
    void hasRunningRootTaskResult(bool running, bool success);
    void lastUpdateTimeResult(const QString &time, bool success);

protected:
    bool ensureInterface();
    const IndexClientDescriptor &descriptor() const;

private:
    void handleHasRunningTaskReply(QDBusPendingCallWatcher *watcher);
    void handleIndexExistsReply(QDBusPendingCallWatcher *watcher);
    void handleServiceTestReply(QDBusPendingCallWatcher *watcher);
    void handleGetLastUpdateTimeReply(QDBusPendingCallWatcher *watcher);
    TaskType stringToTaskType(const QString &type) const;
    bool isSupportedTaskType(const QString &type) const;

private Q_SLOTS:
    void onDBusTaskFinished(const QString &type, const QString &path, bool success);
    void onDBusTaskProgressChanged(const QString &type, const QString &path, qlonglong count, qlonglong total);

private:
    IndexClientDescriptor m_descriptor;
    std::unique_ptr<QDBusAbstractInterface> interface;
};

DPSEARCH_END_NAMESPACE

Q_DECLARE_METATYPE(dfmplugin_search::AbstractIndexClient::TaskType)

#endif   // ABSTRACTINDEXCLIENT_H
