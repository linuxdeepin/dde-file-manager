// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractindexcontroller.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDir>
#include <QTimer>

#include <mutex>

DFMBASE_USE_NAMESPACE
DAEMONPCORE_BEGIN_NAMESPACE

// 保护插件服务只被启动一次，两个控制器共享同一个进程
static std::once_flag s_pluginStartupOnceFlag;

AbstractIndexController::AbstractIndexController(IndexControllerDescriptor descriptor, QObject *parent)
    : QObject(parent),
      m_descriptor(std::move(descriptor)),
      keepAliveTimer(new QTimer(this))
{
    setupStateHandlers();
}

AbstractIndexController::~AbstractIndexController() = default;

void AbstractIndexController::initialize()
{
    fmInfo() << "[" << m_descriptor.controllerName << "] Initializing index controller";
    QString err;
    if (!DConfigManager::instance()->addConfig(m_descriptor.configPath, &err)) {
        fmWarning() << "[" << m_descriptor.controllerName << "] Failed to register search config:" << err;
        return;
    }

    isConfigEnabled = DConfigManager::instance()->value(m_descriptor.configPath, m_descriptor.enableKey).toBool();
    fmInfo() << "[" << m_descriptor.controllerName << "] Config registered successfully, enabled:" << isConfigEnabled;

    keepAliveTimer->setInterval(5 * 60 * 1000);
    updateKeepAliveTimer();

    // 监听索引服务的 DBus 注册/注销：服务异常退出后重新上线时，立即按 dconfig
    // 开关恢复 SetEnabled，而不是等待 5 分钟一次的保活轮询。只恢复监控使能与
    // 状态机，不主动触发重索引——停机期间丢失变更的补偿由服务端自愈机制决定
    // （filename 索引有哨兵恢复；全文/OCR 索引设计上接受 Clean 状态的停机窗口）
    serviceWatcher = new QDBusServiceWatcher(m_descriptor.dbusServiceName,
                                             QDBusConnection::sessionBus(),
                                             QDBusServiceWatcher::WatchForRegistration
                                                     | QDBusServiceWatcher::WatchForUnregistration,
                                             this);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, &AbstractIndexController::handleServiceRegistered);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &AbstractIndexController::handleServiceUnregistered);

    if (isConfigEnabled) {
        activeBackend(true);
    }

    connect(keepAliveTimer, &QTimer::timeout, this, [this]() {
        keepBackendAlive();
    });
    connect(DConfigManager::instance(), &DConfigManager::valueChanged,
            this, [this](const QString &config, const QString &key) {
                handleConfigChanged(config, key);
            });
}

QStringList AbstractIndexController::indexedPaths() const
{
    if (m_descriptor.indexedPathsProvider) {
        return m_descriptor.indexedPathsProvider();
    }

    return {};
}

const IndexControllerDescriptor &AbstractIndexController::descriptor() const
{
    return m_descriptor;
}

void AbstractIndexController::onTaskFinished(const QString &type, const QString &path, bool success)
{
    if (!isTrackedPath(path)) {
        return;
    }

    fmDebug() << "[" << m_descriptor.controllerName << "] Received TaskFinished signal - type:" << type
              << "path:" << path << "success:" << success;
    if (auto handler = taskFinishHandlers.find(currentState); handler != taskFinishHandlers.end()) {
        handler->second(success);
    }
}

void AbstractIndexController::onTaskProgressChanged(const QString &type, const QString &path, qint64 count, qint64 total)
{
    Q_UNUSED(type)
    Q_UNUSED(count)
    Q_UNUSED(total)

    if (!isTrackedPath(path)) {
        return;
    }

    if (currentState == State::Running) {
        return;
    }

    fmDebug() << "[" << m_descriptor.controllerName << "] Task progress changed, transitioning to Running state";
    updateState(State::Running);
    if (!isConfigEnabled && interface) {
        fmInfo() << "[" << m_descriptor.controllerName << "] Service disabled during task execution, stopping task";
        interface->asyncCall(QStringLiteral("StopCurrentTask"));
    }
}

void AbstractIndexController::setupStateHandlers()
{
    stateHandlers[State::Disabled] = [this](bool enable) {
        fmDebug() << "[" << m_descriptor.controllerName << "] Processing Disabled state with enable flag:" << enable;
        if (enable) {
            updateState(State::Idle);
            stateHandlers[State::Idle](true);
        }
    };

    stateHandlers[State::Idle] = [this](bool enable) {
        fmDebug() << "[" << m_descriptor.controllerName << "] Processing Idle state with enable flag:" << enable;
        if (!enable) {
            fmInfo() << "[" << m_descriptor.controllerName << "] Service disabled, transitioning to Disabled state";
            updateState(State::Disabled);
            return;
        }

        if (!isBackendAvaliable()) {
            fmDebug() << "[" << m_descriptor.controllerName << "] Setting up DBus connections in Idle state";
            setupDBusConnections();
        }

        fmDebug() << "[" << m_descriptor.controllerName << "] Checking index database existence";
        QDBusPendingCall pendingCall = interface->asyncCall(QStringLiteral("IndexDatabaseExists"));
        auto *watcher = new QDBusPendingCallWatcher(pendingCall, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher](QDBusPendingCallWatcher *) {
            QDBusPendingReply<bool> reply = *watcher;
            if (reply.isError()) {
                fmWarning() << "[" << m_descriptor.controllerName << "] Failed to check index database existence:"
                            << reply.error().message();
            } else {
                startIndexTask(!reply.value());
            }
            watcher->deleteLater();
        });
    };

    stateHandlers[State::Running] = [this](bool enable) {
        fmDebug() << "[" << m_descriptor.controllerName << "] Processing Running state with enable flag:" << enable;
        if (!enable) {
            if (interface) {
                fmInfo() << "[" << m_descriptor.controllerName << "] Stopping current task due to service disable request";
                interface->asyncCall(QStringLiteral("StopCurrentTask"));
            }
            updateState(State::Disabled);
        }
    };

    taskFinishHandlers[State::Running] = [this](bool success) {
        fmDebug() << "[" << m_descriptor.controllerName << "] Task finished with success:" << success;
        if (success) {
            updateState(State::Idle);
        } else {
            updateState(State::Disabled);
        }
    };
}

void AbstractIndexController::setupDBusConnections()
{
    fmDebug() << "[" << m_descriptor.controllerName << "] Setting up DBus connections to index service";

    // TextIndex 和 OcrIndex 共用同一个插件进程，只需其中一个控制器启动服务即可
    std::call_once(s_pluginStartupOnceFlag, [this]() {
        QDBusConnectionInterface *sessionBusIface = QDBusConnection::sessionBus().interface();
        if (sessionBusIface) {
            sessionBusIface->startService(m_descriptor.dbusServiceName);
        }
    });

    if (!m_descriptor.interfaceFactory) {
        fmWarning() << "[" << m_descriptor.controllerName << "] Missing DBus interface factory";
        return;
    }

    interface.reset(m_descriptor.interfaceFactory(this));
    if (!interface) {
        fmWarning() << "[" << m_descriptor.controllerName << "] Failed to create DBus interface";
        return;
    }

    connect(interface.data(), SIGNAL(TaskFinished(QString, QString, bool)),
            this, SLOT(onTaskFinished(QString, QString, bool)));
    connect(interface.data(), SIGNAL(TaskProgressChanged(QString, QString, qlonglong, qlonglong)),
            this, SLOT(onTaskProgressChanged(QString, QString, qlonglong, qlonglong)));

    fmInfo() << "[" << m_descriptor.controllerName << "] DBus connections established successfully";
}

void AbstractIndexController::startIndexTask(bool isCreate)
{
    if (!interface) {
        fmWarning() << "[" << m_descriptor.controllerName << "] Cannot start index task, DBus interface not available";
        return;
    }

    const QStringList paths = indexedPaths();
    const QString method = isCreate ? QStringLiteral("CreateIndexTask") : QStringLiteral("UpdateIndexTask");

    fmInfo() << "[" << m_descriptor.controllerName << "] Starting" << (isCreate ? "CREATE" : "UPDATE")
             << "index task for directory:" << paths;

    QDBusPendingCall pendingCall = interface->asyncCall(method, QVariant::fromValue(paths), QVariantMap());
    auto *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher](QDBusPendingCallWatcher *) {
        QDBusPendingReply<bool> reply = *watcher;
        if (reply.isError()) {
            fmWarning() << "[" << m_descriptor.controllerName << "] Failed to start index task:" << reply.error().message();
        } else if (reply.value()) {
            fmInfo() << "[" << m_descriptor.controllerName << "] Index task started successfully, transitioning to Running state";
            updateState(State::Running);
        } else {
            fmWarning() << "[" << m_descriptor.controllerName << "] Index task start request returned false";
        }
        watcher->deleteLater();
    });
}

void AbstractIndexController::updateState(State newState)
{
    if (currentState != newState) {
        fmInfo() << "[" << m_descriptor.controllerName << "] State transition:"
                 << static_cast<int>(currentState) << "->" << static_cast<int>(newState);
        currentState = newState;
    }
}

void AbstractIndexController::handleConfigChanged(const QString &config, const QString &key)
{
    if (config == m_descriptor.configPath && key == m_descriptor.enableKey) {
        const bool newEnabled = DConfigManager::instance()->value(config, key).toBool();
        fmInfo() << "[" << m_descriptor.controllerName << "] Config changed from" << isConfigEnabled << "to" << newEnabled;
        isConfigEnabled = newEnabled;
        activeBackend();
        updateKeepAliveTimer();

        if (auto handler = stateHandlers.find(currentState); handler != stateHandlers.end()) {
            handler->second(isConfigEnabled);
        }
    }
}

void AbstractIndexController::handleServiceRegistered()
{
    if (!isConfigEnabled) {
        fmDebug() << "[" << m_descriptor.controllerName
                  << "] Index service registered but config disabled, keep backend disabled";
        return;
    }

    fmInfo() << "[" << m_descriptor.controllerName << "] Index service registered, restoring enabled state";
    // 旧进程遗留的状态（如 Running）已失效：任务随进程消亡，TaskFinished 不会再到达。
    // 仅复位状态机，不触发 Idle handler 的索引库检查/重索引。
    updateState(State::Idle);
    activeBackend(true);
}

void AbstractIndexController::handleServiceUnregistered()
{
    fmWarning() << "[" << m_descriptor.controllerName << "] Index service unregistered from DBus";
    // 服务消失后任务不可能再完成，复位状态机；重新拉起由 systemd Restart /
    // DBus activation（保活轮询的调用即可触发）完成，注册后走恢复流程。
    updateState(State::Disabled);
}

void AbstractIndexController::activeBackend(bool isInit)
{
    if (!isBackendAvaliable()) {
        fmWarning() << "[" << m_descriptor.controllerName << "] Cannot activate backend, DBus interface not available";
        return;
    }

    if (isInit) {
        // 延迟 2s 后发送 SetEnabled，等待插件服务真正启动注册完成
        QTimer::singleShot(2000, this, [this]() {
            if (interface) {
                interface->asyncCall(QStringLiteral("SetEnabled"), isConfigEnabled);
            }
        });
    } else {
        interface->asyncCall(QStringLiteral("SetEnabled"), isConfigEnabled);
    }
}

void AbstractIndexController::keepBackendAlive()
{
    if (!isBackendAvaliable()) {
        fmWarning() << "[" << m_descriptor.controllerName << "] Cannot check backend status, DBus interface not available";
        return;
    }

    QDBusPendingCall pendingCall = interface->asyncCall(QStringLiteral("IsEnabled"));
    auto *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher](QDBusPendingCallWatcher *) {
        QDBusMessage reply = watcher->reply();
        if (reply.type() == QDBusMessage::ErrorMessage) {
            fmWarning() << "[" << m_descriptor.controllerName << "] Failed to query backend enabled state:" << reply.errorMessage();
        } else {
            bool enabled = reply.arguments().isEmpty() ? true : reply.arguments().at(0).toBool();
            if (!enabled && isConfigEnabled) {
                fmWarning() << "[" << m_descriptor.controllerName << "] Backend is disabled but config requires it enabled, reactivating backend";
                activeBackend();
            }
        }
        watcher->deleteLater();
    });
}

bool AbstractIndexController::isBackendAvaliable()
{
    if (!interface) {
        setupDBusConnections();
    }

    return interface != nullptr;
}

void AbstractIndexController::updateKeepAliveTimer()
{
    if (isConfigEnabled && !keepAliveTimer->isActive()) {
        keepAliveTimer->start();
    } else if (!isConfigEnabled && keepAliveTimer->isActive()) {
        keepAliveTimer->stop();
    }
}

bool AbstractIndexController::isTrackedPath(const QString &path) const
{
    const QStringList paths = indexedPaths();
    if (paths.isEmpty()) {
        return path == QDir::homePath();
    }

    return paths.contains(path);
}

DAEMONPCORE_END_NAMESPACE
