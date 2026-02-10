// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXCLIENT_H
#define TEXTINDEXCLIENT_H

#include "dfmplugin_search_global.h"

#include <QObject>
#include <QDBusPendingCallWatcher>
#include <memory>
#include <optional>

class OrgDeepinFilemanagerTextIndexInterface;

DPSEARCH_BEGIN_NAMESPACE

class TextIndexClient : public QObject
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
        Available,   // 服务正常可用
        Unavailable,   // 服务不可用
        Error   // 服务出错
    };
    Q_ENUM(ServiceStatus)

    static TextIndexClient *instance();

    // 异步方法，通过信号返回结果
    void startTask(TaskType type, const QStringList &paths);

    // 异步检查索引是否存在，结果通过信号 indexExistsResult 返回
    void checkIndexExists();

    // 异步检查服务状态，结果通过信号 serviceStatusResult 返回
    void checkServiceStatus();

    // 异步检查根目录("/")的索引任务是否正在运行，结果通过信号 hasRunningRootTaskResult 返回
    void checkHasRunningRootTask();

    // 异步检查是否有任务在运行，结果通过信号 hasRunningTaskResult 返回
    void checkHasRunningTask();

    // 异步获取最后更新时间，结果通过信号 lastUpdateTimeResult 返回
    void getLastUpdateTime();

    void setEnable(bool enabled);

Q_SIGNALS:
    void taskStarted(TaskType type, const QString &path);
    void taskFinished(TaskType type, const QString &path, bool success);
    void taskFailed(TaskType type, const QString &path, const QString &error);
    void taskProgressChanged(TaskType type, const QString &path, qlonglong count, qlonglong total);

    // 新增信号用于异步返回结果
    void indexExistsResult(bool exists, bool success);
    void serviceStatusResult(ServiceStatus status);
    void hasRunningTaskResult(bool running, bool success);
    void hasRunningRootTaskResult(bool running, bool success);
    void lastUpdateTimeResult(const QString &time, bool success);

private:
    explicit TextIndexClient(QObject *parent = nullptr);
    ~TextIndexClient();
    bool ensureInterface();

    // 处理DBus回调的私有方法
    void handleHasRunningTaskReply(QDBusPendingCallWatcher *watcher);
    void handleIndexExistsReply(QDBusPendingCallWatcher *watcher);
    void handleServiceTestReply(QDBusPendingCallWatcher *watcher);
    void handleGetLastUpdateTimeReply(QDBusPendingCallWatcher *watcher);

    // 工具方法：将字符串转换为TaskType
    TaskType stringToTaskType(const QString &type);

    // 工具方法：检查任务类型是否支持
    bool isSupportedTaskType(const QString &type);

private:
    std::unique_ptr<OrgDeepinFilemanagerTextIndexInterface> interface;

private Q_SLOTS:
    void onDBusTaskFinished(const QString &type, const QString &path, bool success);
    void onDBusTaskProgressChanged(const QString &type, const QString &path, qlonglong count, qlonglong total);
};

DPSEARCH_END_NAMESPACE

Q_DECLARE_METATYPE(dfmplugin_search::TextIndexClient::TaskType)

#endif   // TEXTINDEXCLIENT_H
