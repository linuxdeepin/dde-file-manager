// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
        fmDebug() << "TextIndex meta type registered with id:" << id;
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
        } else {
            fmDebug() << "TextIndex service already registered";
        }

        // 创建接口
        fmDebug() << "Creating new TextIndex D-Bus interface";
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
    } else {
        fmDebug() << "TextIndex interface already available and valid";
    }

    return interface && interface->isValid();
}

void TextIndexClient::checkServiceStatus()
{
    if (!ensureInterface()) {
        fmWarning() << "Cannot check service status: interface unavailable";
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
        fmWarning() << "TextIndex service test failed:" << reply.error().message();
        emit serviceStatusResult(ServiceStatus::Error);
    } else {
        fmDebug() << "TextIndex service is available and responding";
        emit serviceStatusResult(ServiceStatus::Available);
    }
}

void TextIndexClient::checkIndexExists()
{
    if (!ensureInterface()) {
        fmWarning() << "Cannot check index existence: interface unavailable";
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
        fmWarning() << "Failed to check if TextIndex database exists:" << reply.error().message();
        emit indexExistsResult(false, false);
    } else {
        emit indexExistsResult(reply.value(), true);
    }
}

void TextIndexClient::startTask(TaskType type, const QStringList &paths)
{
    if (!ensureInterface()) {
        fmCritical() << "Cannot start TextIndex task: interface unavailable";
        emit taskFailed(type, paths.join("|"), "Failed to connect to service");
        return;
    }

    // 直接启动任务，不检查是否有任务在运行
    QDBusPendingReply<bool> pendingTask;
    switch (type) {
    case TaskType::Create:
        fmDebug() << "Sending CreateIndexTask request for paths:" << paths;
        pendingTask = interface->CreateIndexTask(paths);
        break;
    case TaskType::Update:
        fmDebug() << "Sending UpdateIndexTask request for paths:" << paths;
        pendingTask = interface->UpdateIndexTask(paths);
        break;
    default:
        fmWarning() << "Unknown TextIndex task type:" << static_cast<int>(type);
        return;
    }

    auto taskWatcher = new QDBusPendingCallWatcher(pendingTask, this);
    connect(taskWatcher, &QDBusPendingCallWatcher::finished, this, [this, type, paths](QDBusPendingCallWatcher *watcher) {
        FinallyUtil finaly([watcher]() {
            watcher->deleteLater();
        });
        QDBusPendingReply<bool> reply = *watcher;

        if (reply.isError() || !reply.value()) {
            QString errorMsg = reply.isError() ? reply.error().message() : "Failed to start task";
            fmWarning() << "TextIndex task start failed:" << errorMsg;
            emit taskFailed(type, paths.join("|"), errorMsg);
            return;
        }

        emit taskStarted(type, paths.join("|"));
    });
}

// 添加一个私有辅助方法用于检查任务类型是否支持
bool TextIndexClient::isSupportedTaskType(const QString &type)
{
    static const QStringList supportedTypes = {
        "create", "update", "create-file-list", "update-file-list", "remove-file-list", "move-file-list"
    };
    return supportedTypes.contains(type);
}

TextIndexClient::TaskType TextIndexClient::stringToTaskType(const QString &type)
{
    static const QMap<QString, TaskType> typeMap = {
        { "create", TaskType::Create },
        { "update", TaskType::Update },
        { "create-file-list", TaskType::CreateFileList },
        { "update-file-list", TaskType::UpdateFileList },
        { "remove-file-list", TaskType::RemoveFileList },
        { "move-file-list", TaskType::MoveFileList }
    };

    if (!typeMap.contains(type)) {
        fmWarning() << "Unknown TextIndex task type string:" << type << "using default Create type";
        return TaskType::Create;   // 默认返回类型
    }
    return typeMap.value(type);
}

void TextIndexClient::onDBusTaskFinished(const QString &type, const QString &path, bool success)
{
    // 检查是否为支持的类型
    if (!isSupportedTaskType(type))
        return;

    TaskType taskType = stringToTaskType(type);

    if (success) {
        fmDebug() << "TextIndex task completed successfully:" << type << "path:" << path;
        emit taskFinished(taskType, path, true);
    } else {
        fmWarning() << "TextIndex task failed:" << type << "path:" << path;
        emit taskFailed(taskType, path, "Task failed");
    }
}

void TextIndexClient::onDBusTaskProgressChanged(const QString &type, const QString &path, qlonglong count, qlonglong total)
{
    // 检查是否为支持的类型
    if (!isSupportedTaskType(type))
        return;

    TaskType taskType = stringToTaskType(type);
    emit taskProgressChanged(taskType, path, count, total);
}

void TextIndexClient::checkHasRunningTask()
{
    if (!ensureInterface()) {
        fmWarning() << "Cannot check running tasks: interface unavailable";
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
        fmWarning() << "Cannot check running root tasks: interface unavailable";
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
            fmWarning() << "Failed to check TextIndex root task:" << reply.error().message();
            emit hasRunningRootTaskResult(false, false);
            return;
        }

        // 判断正在运行的是否是根任务
        bool isRunning = reply.value();
        bool isRootTask = isRunning;
        emit hasRunningRootTaskResult(isRootTask, true);
    });
}

void TextIndexClient::getLastUpdateTime()
{
    if (!ensureInterface()) {
        fmWarning() << "Cannot get last update time: interface unavailable";
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
    } else {
        fmWarning() << "Cannot set TextIndex enabled state: interface unavailable";
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
