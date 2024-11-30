// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexclient.h"
#include "textindex_interface.h"

DPSEARCH_USE_NAMESPACE

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

        // 验证接口是否真正可用
        auto testReply = interface->HasRunningTask();
        testReply.waitForFinished();
        if (testReply.isError())
            fmWarning() << "[TextIndex] Interface test failed:" << testReply.error().message();

        // 接口可用，连接信号
        connect(interface.get(), &OrgDeepinFilemanagerTextIndexInterface::TaskFinished,
                this, &TextIndexClient::onDBusTaskFinished);
        connect(interface.get(), &OrgDeepinFilemanagerTextIndexInterface::TaskProgressChanged,
                this, &TextIndexClient::onDBusTaskProgressChanged);

        fmInfo() << "[TextIndex] Interface successfully initialized";
    }

    return interface && interface->isValid();
}

TextIndexClient::ServiceStatus TextIndexClient::checkService()
{
    if (!ensureInterface())
        return ServiceStatus::Unavailable;

    // 尝试一个简单的调用来验证服务是否正常工作
    auto pendingHasTask = interface->HasRunningTask();
    pendingHasTask.waitForFinished();

    if (pendingHasTask.isError())
        return ServiceStatus::Error;

    return ServiceStatus::Available;
}

std::optional<bool> TextIndexClient::indexExists()
{
    if (!ensureInterface())
        return std::nullopt;

    auto pendingExists = interface->IndexDatabaseExists();
    pendingExists.waitForFinished();

    if (pendingExists.isError())
        return std::nullopt;

    return pendingExists.value();
}

void TextIndexClient::startTask(TaskType type, const QString &path)
{
    if (!ensureInterface()) {
        emit taskFailed(type, path, "Failed to connect to service");
        return;
    }

    // 检查是否有任务在运行
    auto pendingHasTask = interface->HasRunningTask();
    pendingHasTask.waitForFinished();
    if (pendingHasTask.isError() || pendingHasTask.value()) {
        emit taskFailed(type, path, "Another task is running");
        return;
    }

    // 启动任务
    QDBusPendingReply<bool> pendingTask;
    if (type == TaskType::Create) {
        pendingTask = interface->CreateIndexTask(path);
    } else {
        pendingTask = interface->UpdateIndexTask(path);
    }

    pendingTask.waitForFinished();
    if (pendingTask.isError() || !pendingTask.value()) {
        emit taskFailed(type, path,
                        pendingTask.isError() ? pendingTask.error().message() : "Failed to start task");
        return;
    }

    emit taskStarted(type, path);
    runningTaskPath = path;
}

void TextIndexClient::onDBusTaskFinished(const QString &type, const QString &path, bool success)
{
    TaskType taskType = type == "create" ? TaskType::Create : TaskType::Update;
    if (success) {
        emit taskFinished(taskType, path, true);
    } else {
        emit taskFailed(taskType, path, "Task failed");
    }
    runningTaskPath.clear();
}

void TextIndexClient::onDBusTaskProgressChanged(const QString &type, const QString &path, qlonglong count)
{
    TaskType taskType = type == "create" ? TaskType::Create : TaskType::Update;
    emit taskProgressChanged(taskType, path, count);
}

std::optional<bool> TextIndexClient::hasRunningRootTask()
{
    if (!ensureInterface())
        return std::nullopt;

    auto pendingHasTask = interface->HasRunningTask();
    pendingHasTask.waitForFinished();

    if (pendingHasTask.isError())
        return std::nullopt;

    return pendingHasTask.value() && runningTaskPath == "/";
}
