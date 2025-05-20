// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexclient.h"
#include "textindex_interface.h"

#include <dfm-base/utils/finallyutil.h>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {
void registerMetaTypes()
{
    static bool registered = false;
    if (!registered) {
        int id = qRegisterMetaType<DPSEARCH_NAMESPACE::TextIndexClient::TaskType>("DPSEARCH_NAMESPACE::TextIndexClient::TaskType");
        fmDebug() << "Meta type registered with id:" << id;
        registered = true;
    }
}
}

TextIndexClient *TextIndexClient::instance()
{
    static TextIndexClient instance;
    return &instance;
}

TextIndexClient::TextIndexClient(QObject *parent)
    : QObject(parent)
{
    registerMetaTypes();
}

TextIndexClient::~TextIndexClient()
{
}

bool TextIndexClient::ensureInterface()
{
    if (!interface || !interface->isValid()) {
        Q_ASSERT(qApp->thread() == QThread::currentThread());

        // 先尝试启动服务
        QDBusConnectionInterface *sessionBusIface = QDBusConnection::sessionBus().interface();
        if (!sessionBusIface) {
            fmWarning() << "[TextIndex] Failed to get session bus interface";
            return false;
        }

        if (!sessionBusIface->isServiceRegistered("org.deepin.Filemanager.TextIndex")) {
            auto reply = sessionBusIface->startService("org.deepin.Filemanager.TextIndex");
            if (!reply.isValid())
                fmWarning() << "[TextIndex] Failed to start service:" << reply.error().message();
        }

        // 创建接口
        interface.reset(new OrgDeepinFilemanagerTextIndexInterface(
                "org.deepin.Filemanager.TextIndex",
                "/org/deepin/Filemanager/TextIndex",
                QDBusConnection::sessionBus(),
                this));

        // 检查接口是否有效
        if (!interface->isValid()) {
            fmWarning() << "[TextIndex] Failed to create valid interface:" << interface->lastError().message();
            interface.reset();
            return false;
        }

        // 接口可用，连接信号
        connect(interface.get(), &OrgDeepinFilemanagerTextIndexInterface::TaskFinished,
                this, &TextIndexClient::onDBusTaskFinished);
        connect(interface.get(), &OrgDeepinFilemanagerTextIndexInterface::TaskProgressChanged,
                this, &TextIndexClient::onDBusTaskProgressChanged);

        fmInfo() << "[TextIndex] Interface successfully initialized";
    }

    return interface && interface->isValid();
}

void TextIndexClient::checkServiceStatus()
{
    if (!ensureInterface()) {
        emit serviceStatusResult(ServiceStatus::Unavailable);
        return;
    }

    // 异步调用来验证服务是否正常工作
    auto pendingHasTask = interface->HasRunningTask();
    auto watcher = new QDBusPendingCallWatcher(pendingHasTask, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &TextIndexClient::handleServiceTestReply);
}

void TextIndexClient::handleServiceTestReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        emit serviceStatusResult(ServiceStatus::Error);
    } else {
        emit serviceStatusResult(ServiceStatus::Available);
    }
}

void TextIndexClient::checkIndexExists()
{
    if (!ensureInterface()) {
        emit indexExistsResult(false, false);
        return;
    }

    auto pendingExists = interface->IndexDatabaseExists();
    auto watcher = new QDBusPendingCallWatcher(pendingExists, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &TextIndexClient::handleIndexExistsReply);
}

void TextIndexClient::handleIndexExistsReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        emit indexExistsResult(false, false);
    } else {
        emit indexExistsResult(reply.value(), true);
    }
}

void TextIndexClient::startTask(TaskType type, const QStringList &paths)
{
    if (!ensureInterface()) {
        emit taskFailed(type, paths.join("|"), "Failed to connect to service");
        return;
    }

    // 异步检查是否有任务在运行
    auto pendingHasTask = interface->HasRunningTask();
    auto watcher = new QDBusPendingCallWatcher(pendingHasTask, this);

    // 使用lambda捕获任务类型和路径信息，在回调中处理
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, type, paths](QDBusPendingCallWatcher *watcher) {
        this->handleTaskStartReply(watcher, type, paths);
    });
}

void TextIndexClient::handleTaskStartReply(QDBusPendingCallWatcher *watcher, TaskType type, const QStringList &paths)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError() || reply.value()) {
        emit taskFailed(type, paths.join("|"),
                        reply.isError() ? reply.error().message() : "Another task is running");
        return;
    }

    // 异步启动任务
    QDBusPendingReply<bool> pendingTask;
    switch (type) {
    case TaskType::Create:
        pendingTask = interface->CreateIndexTask(paths);
        break;
    case TaskType::Update:
        pendingTask = interface->UpdateIndexTask(paths);
        break;
    default:
        fmWarning() << "Unknown task type:" << static_cast<int>(type);
        return;
    }

    auto taskWatcher = new QDBusPendingCallWatcher(pendingTask, this);
    connect(taskWatcher, &QDBusPendingCallWatcher::finished, this, [this, type, paths](QDBusPendingCallWatcher *watcher) {
        FinallyUtil finaly([watcher]() {
            watcher->deleteLater();
        });
        QDBusPendingReply<bool> reply = *watcher;

        if (reply.isError() || !reply.value()) {
            emit taskFailed(type, paths.join("|"),
                            reply.isError() ? reply.error().message() : "Failed to start task");
            return;
        }

        emit taskStarted(type, paths.join("|"));
        runningTaskPath = paths.join("|");
    });
}

void TextIndexClient::onDBusTaskFinished(const QString &type, const QString &path, bool success)
{
    TaskType taskType;
    if (type == "create")
        taskType = TaskType::Create;
    else if (type == "update")
        taskType = TaskType::Update;
    else if (type == "remove")
        taskType = TaskType::Remove;
    else
        return;

    if (success) {
        emit taskFinished(taskType, path, true);
    } else {
        emit taskFailed(taskType, path, "Task failed");
    }
    runningTaskPath.clear();
}

void TextIndexClient::onDBusTaskProgressChanged(const QString &type, const QString &path, qlonglong count, qlonglong total)
{
    TaskType taskType;
    if (type == "create")
        taskType = TaskType::Create;
    else if (type == "update")
        taskType = TaskType::Update;
    else if (type == "remove")
        taskType = TaskType::Remove;
    else
        return;

    emit taskProgressChanged(taskType, path, count, total);
}

void TextIndexClient::checkHasRunningTask()
{
    if (!ensureInterface()) {
        emit hasRunningTaskResult(false, false);
        return;
    }

    auto pendingHasTask = interface->HasRunningTask();
    auto watcher = new QDBusPendingCallWatcher(pendingHasTask, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &TextIndexClient::handleHasRunningTaskReply);
}

void TextIndexClient::handleHasRunningTaskReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        fmWarning() << "[TextIndex] Failed to check running task:" << reply.error().message();
        emit hasRunningTaskResult(false, false);
    } else {
        emit hasRunningTaskResult(reply.value(), true);
    }
}

void TextIndexClient::checkHasRunningRootTask()
{
    if (!ensureInterface()) {
        emit hasRunningRootTaskResult(false, false);
        return;
    }

    // 先检查是否有任务运行
    auto pendingHasTask = interface->HasRunningTask();
    auto watcher = new QDBusPendingCallWatcher(pendingHasTask, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        FinallyUtil finaly([watcher]() {
            watcher->deleteLater();
        });
        QDBusPendingReply<bool> reply = *watcher;

        if (reply.isError()) {
            emit hasRunningRootTaskResult(false, false);
            return;
        }

        // 判断正在运行的是否是根任务
        bool isRunning = reply.value();
        bool isRootTask = isRunning && runningTaskPath == "/";
        emit hasRunningRootTaskResult(isRootTask, true);
    });
}

void TextIndexClient::getLastUpdateTime()
{
    if (!ensureInterface()) {
        emit lastUpdateTimeResult(QString(), false);
        return;
    }

    auto pendingReply = interface->GetLastUpdateTime();
    auto watcher = new QDBusPendingCallWatcher(pendingReply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &TextIndexClient::handleGetLastUpdateTimeReply);
}

void TextIndexClient::setEnable(bool enabled)
{
    if (ensureInterface()) {
        interface->SetEnabled(enabled);
    }
}

void TextIndexClient::handleGetLastUpdateTimeReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<QString> reply = *watcher;

    if (reply.isError()) {
        fmWarning() << "[TextIndex] Get last update time failed:" << reply.error().message();
        emit lastUpdateTimeResult(QString(), false);
    } else {
        emit lastUpdateTimeResult(reply.value(), true);
    }
}
