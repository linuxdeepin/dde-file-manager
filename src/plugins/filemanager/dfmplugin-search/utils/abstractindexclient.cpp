// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractindexclient.h"

#include <dfm-base/utils/finallyutil.h>

#include <QApplication>
#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingReply>
#include <QMap>
#include <QThread>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

AbstractIndexClient::AbstractIndexClient(IndexClientDescriptor descriptor, QObject *parent)
    : QObject(parent),
      m_descriptor(std::move(descriptor))
{
}

AbstractIndexClient::~AbstractIndexClient() = default;

bool AbstractIndexClient::ensureInterface()
{
    if (interface && interface->isValid()) {
        fmDebug() << "[" << m_descriptor.clientName << "] interface already available and valid";
        return true;
    }

    Q_ASSERT(qApp->thread() == QThread::currentThread());

    QDBusConnectionInterface *sessionBusIface = QDBusConnection::sessionBus().interface();
    if (!sessionBusIface) {
        fmWarning() << "[" << m_descriptor.clientName << "] failed to get session bus interface";
        return false;
    }

    if (!sessionBusIface->isServiceRegistered(m_descriptor.dbusServiceName)) {
        auto reply = sessionBusIface->startService(m_descriptor.dbusServiceName);
        if (!reply.isValid()) {
            fmWarning() << "[" << m_descriptor.clientName << "] failed to start service:" << reply.error().message();
        }
    } else {
        fmDebug() << "[" << m_descriptor.clientName << "] service already registered";
    }

    if (!m_descriptor.interfaceFactory) {
        fmWarning() << "[" << m_descriptor.clientName << "] missing DBus interface factory";
        return false;
    }

    fmDebug() << "[" << m_descriptor.clientName << "] creating new D-Bus interface";
    interface.reset(m_descriptor.interfaceFactory(this));
    if (!interface || !interface->isValid()) {
        const QString error = interface ? interface->lastError().message() : QString();
        fmWarning() << "[" << m_descriptor.clientName << "] failed to create valid interface:" << error;
        interface.reset();
        return false;
    }

    connect(interface.get(), SIGNAL(TaskFinished(QString, QString, bool)),
            this, SLOT(onDBusTaskFinished(QString, QString, bool)));
    connect(interface.get(), SIGNAL(TaskProgressChanged(QString, QString, qlonglong, qlonglong)),
            this, SLOT(onDBusTaskProgressChanged(QString, QString, qlonglong, qlonglong)));

    fmInfo() << "[" << m_descriptor.clientName << "] interface successfully initialized";
    return true;
}

const IndexClientDescriptor &AbstractIndexClient::descriptor() const
{
    return m_descriptor;
}

void AbstractIndexClient::checkServiceStatus()
{
    if (!ensureInterface()) {
        fmWarning() << "[" << m_descriptor.clientName << "] cannot check service status: interface unavailable";
        emit serviceStatusResult(ServiceStatus::Unavailable);
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(interface->asyncCall(QStringLiteral("HasRunningTask")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this](QDBusPendingCallWatcher *watcher) {
                handleServiceTestReply(watcher);
            });
}

void AbstractIndexClient::handleServiceTestReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        fmWarning() << "[" << m_descriptor.clientName << "] service test failed:" << reply.error().message();
        emit serviceStatusResult(ServiceStatus::Error);
    } else {
        fmDebug() << "[" << m_descriptor.clientName << "] service is available and responding";
        emit serviceStatusResult(ServiceStatus::Available);
    }
}

void AbstractIndexClient::checkIndexExists()
{
    if (!ensureInterface()) {
        fmWarning() << "[" << m_descriptor.clientName << "] cannot check index existence: interface unavailable";
        emit indexExistsResult(false, false);
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(interface->asyncCall(QStringLiteral("IndexDatabaseExists")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this](QDBusPendingCallWatcher *watcher) {
                handleIndexExistsReply(watcher);
            });
}

void AbstractIndexClient::handleIndexExistsReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        fmWarning() << "[" << m_descriptor.clientName << "] failed to check if database exists:" << reply.error().message();
        emit indexExistsResult(false, false);
    } else {
        emit indexExistsResult(reply.value(), true);
    }
}

void AbstractIndexClient::startTask(TaskType type, const QStringList &paths)
{
    if (!ensureInterface()) {
        fmCritical() << "[" << m_descriptor.clientName << "] cannot start task: interface unavailable";
        emit taskFailed(type, paths.join("|"), "Failed to connect to service");
        return;
    }

    QString method;
    switch (type) {
    case TaskType::Create:
        method = QStringLiteral("CreateIndexTask");
        break;
    case TaskType::Update:
        method = QStringLiteral("UpdateIndexTask");
        break;
    default:
        fmWarning() << "[" << m_descriptor.clientName << "] unknown task type:" << static_cast<int>(type);
        return;
    }

    fmDebug() << "[" << m_descriptor.clientName << "] sending" << method << "request for paths:" << paths;
    auto watcher = new QDBusPendingCallWatcher(interface->asyncCall(method, QVariant::fromValue(paths)), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, type, paths](QDBusPendingCallWatcher *watcher) {
                FinallyUtil finaly([watcher]() {
                    watcher->deleteLater();
                });
                QDBusPendingReply<bool> reply = *watcher;

                if (reply.isError() || !reply.value()) {
                    const QString errorMsg = reply.isError() ? reply.error().message() : "Failed to start task";
                    fmWarning() << "[" << m_descriptor.clientName << "] task start failed:" << errorMsg;
                    emit taskFailed(type, paths.join("|"), errorMsg);
                    return;
                }

                emit taskStarted(type, paths.join("|"));
            });
}

bool AbstractIndexClient::isSupportedTaskType(const QString &type) const
{
    static const QStringList supportedTypes {
        QStringLiteral("create"),
        QStringLiteral("update"),
        QStringLiteral("create-file-list"),
        QStringLiteral("update-file-list"),
        QStringLiteral("remove-file-list"),
        QStringLiteral("move-file-list")
    };

    return supportedTypes.contains(type);
}

AbstractIndexClient::TaskType AbstractIndexClient::stringToTaskType(const QString &type) const
{
    static const QMap<QString, TaskType> typeMap {
        { QStringLiteral("create"), TaskType::Create },
        { QStringLiteral("update"), TaskType::Update },
        { QStringLiteral("create-file-list"), TaskType::CreateFileList },
        { QStringLiteral("update-file-list"), TaskType::UpdateFileList },
        { QStringLiteral("remove-file-list"), TaskType::RemoveFileList },
        { QStringLiteral("move-file-list"), TaskType::MoveFileList }
    };

    const auto it = typeMap.find(type);
    if (it == typeMap.end()) {
        fmWarning() << "[" << m_descriptor.clientName << "] unknown task type string:" << type
                    << "using default Create type";
        return TaskType::Create;
    }

    return it.value();
}

void AbstractIndexClient::onDBusTaskFinished(const QString &type, const QString &path, bool success)
{
    if (!isSupportedTaskType(type)) {
        return;
    }

    const TaskType taskType = stringToTaskType(type);
    if (success) {
        fmDebug() << "[" << m_descriptor.clientName << "] task completed successfully:" << type << "path:" << path;
        emit taskFinished(taskType, path, true);
    } else {
        fmWarning() << "[" << m_descriptor.clientName << "] task failed:" << type << "path:" << path;
        emit taskFailed(taskType, path, "Task failed");
    }
}

void AbstractIndexClient::onDBusTaskProgressChanged(const QString &type, const QString &path, qlonglong count, qlonglong total)
{
    if (!isSupportedTaskType(type)) {
        return;
    }

    emit taskProgressChanged(stringToTaskType(type), path, count, total);
}

void AbstractIndexClient::checkHasRunningTask()
{
    if (!ensureInterface()) {
        fmWarning() << "[" << m_descriptor.clientName << "] cannot check running tasks: interface unavailable";
        emit hasRunningTaskResult(false, false);
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(interface->asyncCall(QStringLiteral("HasRunningTask")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this](QDBusPendingCallWatcher *watcher) {
                handleHasRunningTaskReply(watcher);
            });
}

void AbstractIndexClient::handleHasRunningTaskReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<bool> reply = *watcher;

    if (reply.isError()) {
        fmWarning() << "[" << m_descriptor.clientName << "] failed to check running task:" << reply.error().message();
        emit hasRunningTaskResult(false, false);
    } else {
        emit hasRunningTaskResult(reply.value(), true);
    }
}

void AbstractIndexClient::checkHasRunningRootTask()
{
    if (!ensureInterface()) {
        fmWarning() << "[" << m_descriptor.clientName << "] cannot check running root tasks: interface unavailable";
        emit hasRunningRootTaskResult(false, false);
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(interface->asyncCall(QStringLiteral("HasRunningTask")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this](QDBusPendingCallWatcher *watcher) {
                FinallyUtil finaly([watcher]() {
                    watcher->deleteLater();
                });
                QDBusPendingReply<bool> reply = *watcher;

                if (reply.isError()) {
                    fmWarning() << "[" << m_descriptor.clientName << "] failed to check root task:" << reply.error().message();
                    emit hasRunningRootTaskResult(false, false);
                    return;
                }

                emit hasRunningRootTaskResult(reply.value(), true);
            });
}

void AbstractIndexClient::getLastUpdateTime()
{
    if (!ensureInterface()) {
        fmWarning() << "[" << m_descriptor.clientName << "] cannot get last update time: interface unavailable";
        emit lastUpdateTimeResult(QString(), false);
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(interface->asyncCall(QStringLiteral("GetLastUpdateTime")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this](QDBusPendingCallWatcher *watcher) {
                handleGetLastUpdateTimeReply(watcher);
            });
}

void AbstractIndexClient::setEnable(bool enabled)
{
    if (!ensureInterface()) {
        fmWarning() << "[" << m_descriptor.clientName << "] cannot set enabled state: interface unavailable";
        return;
    }

    interface->asyncCall(QStringLiteral("SetEnabled"), enabled);
}

void AbstractIndexClient::handleGetLastUpdateTimeReply(QDBusPendingCallWatcher *watcher)
{
    FinallyUtil finaly([watcher]() {
        watcher->deleteLater();
    });
    QDBusPendingReply<QString> reply = *watcher;

    if (reply.isError()) {
        fmWarning() << "[" << m_descriptor.clientName << "] get last update time failed:" << reply.error().message();
        emit lastUpdateTimeResult(QString(), false);
    } else {
        emit lastUpdateTimeResult(reply.value(), true);
    }
}
